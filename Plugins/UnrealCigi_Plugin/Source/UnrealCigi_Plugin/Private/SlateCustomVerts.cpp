//Copyright SimBlocks LLC 2016-2026

#include "SlateCustomVerts.h"
#include "unrealcigiEventHandler.h"
#include "Runtime\Launch\Resources\Version.h"
#include "unrealcigiUtil.h"
#include <cmath>

DEFINE_LOG_CATEGORY(LogCigiSlateVerts)

VertexTransform::VertexTransform(FVector2D offset, float rotation, FVector2D scale, bool applyParentScale)
{
  Offset = offset;
  Rotation = rotation;
  Scale = scale;
  ApplyParentScale = applyParentScale;
}

SlateCustomVerts::SlateCustomVerts(FVector2D allocatedGeometryPos, FVector2D allocatedGeometrySize)
{
  AllocatedGeometryPos = allocatedGeometryPos.GetAbs();
  AllocatedGeometrySize = allocatedGeometrySize.GetAbs();
  Vertices = TArray<FSlateVertex>();
  Indices = TArray<SlateIndex>();
  RemoveUVGrid();
  Transforms = TArray<VertexTransform>();

  // Determine the aspect ratio of the screen. Set this in the constructor to make sure it is never (0,0)
  if (FMath::IsNearlyZero(AllocatedGeometrySize.X) || FMath::IsNearlyZero(AllocatedGeometrySize.Y))
  {
    AspectRatioScreen = FVector2D(1.0, 1.0);
  }
  else if (AllocatedGeometrySize.X > AllocatedGeometrySize.Y)
  {
    AspectRatioScreen = FVector2D(AllocatedGeometrySize.X / AllocatedGeometrySize.Y, 1.0);
  }
  else if (AllocatedGeometrySize.X < AllocatedGeometrySize.Y)
  {
    AspectRatioScreen = FVector2D(1.0, AllocatedGeometrySize.Y / AllocatedGeometrySize.X);
  }
  else
  {
    AspectRatioScreen = FVector2D(1.0, 1.0);
  }
}

void SlateCustomVerts::SetUVGrid(FVector2D minUV, FVector2D maxUV, bool upPositive)
{
  // Determine the width and height of the UV grid
  FVector2D uvSize = maxUV - minUV;
  if (FMath::IsNearlyZero(uvSize.X) || FMath::IsNearlyZero(uvSize.Y))
  {
    Scale = FVector2D::ZeroVector;
    Offset = AllocatedGeometryPos;
    UpPositive = upPositive;
    return;
  }
  // Determine the offset and scale needed to convert from surface space (UV) to screen space (pixels)
  Scale = AllocatedGeometrySize / uvSize;
  Offset = AllocatedGeometryPos - (minUV * Scale);
  // Fit the aspect ratio of the screen
  Scale *= AspectRatioScreen;
  // Record if positive values need to point upwards (UV origin is bottom-left, screen origin is top-left)
  UpPositive = upPositive;
}

void SlateCustomVerts::RemoveUVGrid()
{
  Scale = FVector2D(1, 1);
  Offset = FVector2D(0, 0);
  UpPositive = false;
}

FVector2D SlateCustomVerts::UVToScreen(FVector2D uv) const
{
  // If SetCoordinates was not run, this does nothing (multiply by 1, add 0)
  FVector2D screenPos = uv * Scale + Offset;
  // if UpPositive is true, un-swap the y-axis (top-curr+origin)
  if (UpPositive)
  {
    screenPos.Y = (AllocatedGeometryPos.Y + AllocatedGeometrySize.Y) - screenPos.Y + AllocatedGeometryPos.Y;
  }

  return screenPos;
}

FVector2D SlateCustomVerts::ApplyTransform(FVector2D uv)
{
  // Scale first, while the points are still in the correct orientation
  // Only use this symbol's scale, all parent scales are ignored (according to CIGI)
  if (Transforms.Num() > 0)
  {
    uv = uv * Transforms[0].Scale;
  }
  // Rotations and offsets are applied in series (according to CIGI)
  for (int i = 0; i < Transforms.Num(); i++)
  {
    VertexTransform vt = Transforms[i];
    // Usually CIGI does not apply parent scales, but there are cases where this is needed (ex: adjusting for aspect ratio)
    if (i > 0 && vt.ApplyParentScale)
    {
      uv = uv * vt.Scale;
    }
    // Rotate, while the origin is still at the (0,0) of this vertex transform
    uv = FQuat2D(vt.Rotation * PI / 180).TransformPoint(uv);
    // Then translate to the offset position
    uv = uv + vt.Offset;
  }
  return uv;
}

FSlateRenderTransform SlateCustomVerts::MakeRenderTransform()
{
  if (Transforms.Num() <= 0)
  {
    return FSlateRenderTransform();
  }
  // Only use the scale of this current symbol and ignore the scaling of parent symbols (according to CIGI)
  // Text scaling must be uniform, so take the average of the U and V scales
  FSlateRenderTransform rt = FSlateRenderTransform((Transforms[0].Scale.X + Transforms[0].Scale.Y) / 2.0f);
  // NOTE: When using Concatenate, the left-hand transform is applied first, then the parameter is applied second
  for (VertexTransform vt : Transforms)
  {
    // Invert the rotation and convert it from degrees to radians
    rt = rt.Concatenate(FSlateRenderTransform(FQuat2D((360.0f - vt.Rotation) * PI / 180)));
    // Convert the offset into screen coordinates
    rt = rt.Concatenate(FSlateRenderTransform(UVToScreen(vt.Offset)));
  }
  return rt;
}

int SlateCustomVerts::IsClockwise(FVector2D a, FVector2D b, FVector2D c)
{
  double result = (b.X - a.X) * (c.Y - a.Y) - (c.X - a.X) * (b.Y - a.Y);
  // Clockwise
  if (result < 0)
  {
    return 1;
  }
  // Counterclockwise
  if (result > 0)
  {
    return -1;
  }
  // Collinear
  return 0;
}

FVector2D SlateCustomVerts::PosOnCircle(float angle, float radius, FVector2D center)
{
  return center + (FVector2D(cos(angle * SBIO_DOUBLE_PI / 180.0), sin(angle * SBIO_DOUBLE_PI / 180.0)) * radius);
}

bool SlateCustomVerts::ReadStipple(uint16_t stipplePattern, int index)
{
  if (stipplePattern <= 0 || stipplePattern == 0xFFFF)
  {
    return true;
  }
  index %= 16;
  if (index < 0)
  {
    index += 16;
  }
  int bit = (stipplePattern >> index) & 1;
  return bit == 1;
}

void SlateCustomVerts::AddVertex(FColor color, FVector2D pos)
{
  // If necessary, transform the point in UV space
  pos = ApplyTransform(pos);

  // Map the uv coordinates to absolute screen coordinates
  pos = UVToScreen(pos);

  // After UE5.0, FSlateVertex started using a different type of position vector
  Vertices.AddZeroed();
#if ENGINE_MAJOR_VERSION < 5
  Vertices.Last().Position = pos;
#else
  Vertices.Last().Position = FVector2f(pos.X, pos.Y);
#endif
  Vertices.Last().Color = color;
  // Indices just counts upwards from zero
  Indices.Add(Indices.Num());
}

void SlateCustomVerts::AddTexturedVertex(FColor color, FVector2D pos, FVector2D textureST)
{
  pos = UVToScreen(ApplyTransform(pos));

  Vertices.AddZeroed();
#if ENGINE_MAJOR_VERSION < 5
  Vertices.Last().Position = pos;
#else
  Vertices.Last().Position = FVector2f(pos.X, pos.Y);
#endif
  Vertices.Last().TexCoords[0] = textureST.X;
  Vertices.Last().TexCoords[1] = textureST.Y;
  Vertices.Last().TexCoords[2] = 1.0f;
  Vertices.Last().TexCoords[3] = 1.0f;
  Vertices.Last().Color = color;
  Indices.Add(Indices.Num());
}

void SlateCustomVerts::AddTriangle(FColor color, FVector2D a, FVector2D b, FVector2D c)
{
  int cw = IsClockwise(a, b, c);
  // If the three points are collinear, then nothing will be displayed anyways so don't bother drawing the vertices
  if (cw == 0)
  {
    return;
  }
  // If the three points are arranged counterclockwise, swap b and c to make the points clockwise
  if (cw < 0)
  {
    FVector2D temp = b;
    b = c;
    c = temp;
  }

  // Create the vertices
  AddVertex(color, a);
  AddVertex(color, b);
  AddVertex(color, c);
}

void SlateCustomVerts::AddTexturedTriangle(FColor color, FVector2D a, FVector2D aST, FVector2D b, FVector2D bST, FVector2D c, FVector2D cST)
{
  const int clockwise = IsClockwise(a, b, c);
  if (clockwise == 0)
  {
    return;
  }
  if (clockwise < 0)
  {
    Swap(b, c);
    Swap(bST, cST);
  }

  AddTexturedVertex(color, a, aST);
  AddTexturedVertex(color, b, bST);
  AddTexturedVertex(color, c, cST);
}

void SlateCustomVerts::AddQuad(FColor color, FVector2D a, FVector2D b, FVector2D c, FVector2D d)
{
  int cw1 = IsClockwise(a, b, c);
  int cw2 = IsClockwise(c, d, a);
  // If abc and cda have opposite orientations, then the quad will not be drawn correctly.
  if ((cw1 < 0 && cw2 > 0) || (cw1 > 0 && cw2 < 0))
  {
    // To fix this, either a must be swapped with either b or d (depending on the orientations)
    if (IsClockwise(b, a, c) == IsClockwise(c, d, b))
    {
      FVector2D temp = a;
      a = b;
      b = temp;
    }
    else
    {
      FVector2D temp = a;
      a = d;
      d = temp;
    }
  }
  // Draw the two triangles for this quad
  AddTriangle(color, a, b, c);
  AddTriangle(color, c, d, a);
}

void SlateCustomVerts::AddCircle(FColor color, FVector2D center, float outerRadius, float innerRadius, float startAngle, float endAngle, uint16_t stipplePattern, float stippleLength, int segmentDegrees)
{
  // ----- Sanitize inputs

  // radius must be positive
  if (outerRadius < 0)
  {
    outerRadius = 0;
  }
  if (innerRadius < 0)
  {
    innerRadius = 0;
  }
  // If the circle has no area, don't bother displaying it
  if (innerRadius == outerRadius)
  {
    return;
  }
  // Un-swap the inner and outer radius, if necessary
  if (innerRadius > outerRadius)
  {
    float swapRadius = innerRadius;
    innerRadius = outerRadius;
    outerRadius = swapRadius;
  }
  // Wrap the angles into range [0,360) (390 will become 30. -45 will become 315, etc.)
  startAngle = fmod(startAngle, 360);
  if (startAngle < 0)
  {
    startAngle += 360;
  }
  endAngle = fmod(endAngle, 360);
  if (endAngle < 0)
  {
    endAngle += 360;
  }
  // There must be a finite number of segments. Default segment length is 10?
  if (segmentDegrees <= 0.1f)
  {
    segmentDegrees = 10;
  }

  // ----- Calculate Segments

  // If end <= start, then the circle has to wrap around past 360?
  if (endAngle <= startAngle)
  {
    endAngle += 360;
  }

  float stippleSegmentDegrees = -1;
  bool hasStipple = stippleLength > 0 && stipplePattern > 0 && stipplePattern != 0xFFFF;
  if (hasStipple)
  {
    // Use the given radius for the line, not the calculated outer edges
    float lineRadius = (outerRadius + innerRadius) / 2.0;
    // Given the arc length of a stipple pattern, determine its angle in degrees along the line
    float stippleDegrees = (360 * segmentDegrees) / (2 * SBIO_DOUBLE_PI * lineRadius);
    // Calculate the degrees of an individual stipple segment (16 segments per stipple)
    stippleSegmentDegrees = stippleDegrees / 16.0f;
  }

  // ----- Draw Segments

  // Draw each segment of the circle
  // (currAngle is distance traveled, currAngle+startAngle is the actual position on the circle)
  float currAngle = 0;
  while (currAngle + startAngle < endAngle - 0.01)
  {
    // Determine end point of this quad
    float targetAngle = -1;
    bool drawQuad = true;
    float segmentTarget = (int)(currAngle / segmentDegrees) * segmentDegrees + segmentDegrees;
    if (hasStipple)
    {
      float stippleIndex = currAngle / stippleSegmentDegrees;
      // If the index is too close to the next index, fix the floating-point error (ex: use "5 / 5 = 1" instead of "4.999 / 5 = 0")
      if ((int)stippleIndex + 1 - stippleIndex < 0.001)
      {
        stippleIndex += 1;
      }
      if (!ReadStipple(stipplePattern, (int)stippleIndex))
      {
        drawQuad = false;
      }
      float stippleTarget = (int)stippleIndex * stippleSegmentDegrees + stippleSegmentDegrees;
      targetAngle = segmentTarget <= stippleTarget ? segmentTarget : stippleTarget;
    }
    else
    {
      targetAngle = segmentTarget;
    }

    if (drawQuad)
    {
      AddQuad(color, PosOnCircle(currAngle + startAngle, outerRadius, center), PosOnCircle(currAngle + startAngle, innerRadius, center), PosOnCircle(targetAngle + startAngle, innerRadius, center), PosOnCircle(targetAngle + startAngle, outerRadius, center));
    }

    // Record the reached target as the new current position
    currAngle = targetAngle;
  }
}

void SlateCustomVerts::AddLine(FColor color, TArray<FVector2D> points, bool connected, bool loop, float lineWidth, uint16_t stipplePattern, float stippleLength)
{
  if (points.Num() < 2)
  {
    return;
  }
  if (lineWidth < 0)
  {
    lineWidth *= -1;
  }
  bool hasStipple = stippleLength > 0 && stipplePattern > 0 && stipplePattern != 0xFFFF;
  float stippleSegmentLength = stippleLength / 16.0;
  // Safeguards against rounding errors in vector math
  float tolerance = 0.0001;

  if (connected && loop)
  {
    FVector2D startPoint = points[0];
    points.Add(startPoint);
  }

  // For each point, calculate the normalized direction towards the next point
  TArray<FVector2D> dirs = TArray<FVector2D>();
  for (int i = 0; i < points.Num(); i++)
  {
    // The normalized direction of this segment
    FVector2D dir = FVector2D(0, 0);
    if (i + 1 < points.Num())
    {
      dir = (points[i + 1] - points[i]).GetSafeNormal();
    }
    else if (connected && loop)
    {
      dir = dirs[0];
    }
    dirs.Add(dir);
  }

  // Records how much distance has been traveled along the line so far
  // Used to determine the current position in the stipple pattern
  float stippleProgress = 0;
  // Draw the line segments (segments are drawn from points[i] to points[i+1])
  for (int i = 0; i < points.Num() - 1; i++)
  {
    // If the lines are not connected, don't draw the "odd" segments
    if (!connected && i % 2 == 1)
    {
      continue;
    }
    FVector2D currPos = points[i];
    float segmentDistance = FVector2D::Distance(currPos, points[i + 1]);
    while (segmentDistance > tolerance)
    {
      UE_LOG(LogCigiSlateVerts, SLATE_VERTS, TEXT("LogCigiCustomVerts: i=%d/%d, starting seg: staying in while loop b/c segDist=%.6f > tol=%.6f"), i, points.Num() - 1, segmentDistance, tolerance);
      // If there is no stipple keep the default values
      float travelDistance = segmentDistance;
      bool drawQuad = true;
      // If there is a stipple, calculate the travel distance and determine if this stipple bit is enabled
      if (hasStipple)
      {
        float stippleDistance = stippleSegmentLength - fmod(stippleProgress, stippleSegmentLength);
        // If the distance is too close to 0, fix the fmod error (ex: use "5 % 5 = 0" instead of "4.9999 % 5 = 4.9999")
        if (stippleDistance < tolerance)
        {
          stippleDistance = 0;
          UE_LOG(LogCigiSlateVerts, SLATE_VERTS, TEXT("LogCigi: SlateCustomVerts::AddLine: Fixed floating-point fmod error"));
        }
        if (stippleDistance < segmentDistance)
        {
          travelDistance = stippleDistance;
        }

        float stippleIndex = stippleProgress / stippleSegmentLength;
        // If the index is too close to the next index, fix the floating-point error (ex: use "5 / 5 = 1" instead of "4.9999 / 5 = 0")
        if ((int)stippleIndex + 1 - stippleIndex < 0.000001)
        {
          stippleIndex += 1;
        }
        if (!ReadStipple(stipplePattern, (int)stippleIndex))
        {
          drawQuad = false;
        }
      }

      FVector2D targetPos = currPos + (dirs[i] * travelDistance);
      UE_LOG(LogCigiSlateVerts, SLATE_VERTS, TEXT("LogCigiCustomVerts: i=%d/%d, starting seg: currPos=(%.2f,%.2f), targetPos:(%.2f,%.2f)"), i, points.Num() - 1, currPos.X, currPos.Y, targetPos.X, targetPos.Y);

      if (drawQuad)
      {
        // The direction and distance to travel from a point at the line's center to the edge of its width
        FVector2D widthDelta = FVector2D(-dirs[i].Y, dirs[i].X) * (lineWidth / 2.0);

        // By default, use the basic line width formula
        FVector2D currInnerPoint = currPos - widthDelta;
        FVector2D currOuterPoint = currPos + widthDelta;
        // If the current point is connected to a previous line segment, use bisector vectors instead
        if (connected && (i > 0 || loop) && FVector2D::Distance(currPos, points[i]) < tolerance)
        {
          int prevDirIndex = i > 0 ? i - 1 : dirs.Num() - 1;

          FVector2D b = (dirs[prevDirIndex] + -dirs[i]).GetSafeNormal();
          float fi = atan2(FVector2D::CrossProduct(b, dirs[prevDirIndex]), FVector2D::DotProduct(b, dirs[prevDirIndex]));
          // If the angle is perfectly equal to 0 (lines are parallel), then just use the regular line width (don't divide by 0)
          if (fi != 0)
          {
            float l = (lineWidth / 2.0) / sin(fi);
            currInnerPoint = currPos + (l * b);
            currOuterPoint = currPos - (l * b);
            UE_LOG(LogCigiSlateVerts, SLATE_VERTS, TEXT("LogCigiCustomVerts: i=%d/%d, curr=FANCY from dirs[%d]=(%.3f,%.3f) to -dirs[%d]=(%.3f,%.3f) using fi=%.3f and l=%.3f, created inner=(%.3f,%.3f) and outer=(%.3f,%.3f)"), i, points.Num() - 1, prevDirIndex, dirs[prevDirIndex].X,
                   dirs[prevDirIndex].Y, i, (-dirs[i]).X, (-dirs[i]).Y, fi, l, currInnerPoint.X, currInnerPoint.Y, currOuterPoint.X, currOuterPoint.Y);
          }
        }
        else
        {
          UE_LOG(LogCigiSlateVerts, SLATE_VERTS, TEXT("LogCigiCustomVerts: i=%d/%d, curr=NORMAL"), i, points.Num() - 1);
        }

        // By default, use the basic line width formula
        FVector2D targetInnerPoint = targetPos - widthDelta;
        FVector2D targetOuterPoint = targetPos + widthDelta;
        // If the target point is connected to the next line segment, use bisector vectors instead
        if (connected && FVector2D::Distance(targetPos, points[i + 1]) < tolerance)
        {
          // Unit bisector vector
          FVector2D ab = dirs[i];
          FVector2D cb = -dirs[i + 1];
          FVector2D b = (ab + cb).GetSafeNormal();
          // Angle between b and ab
          float fi = atan2(FVector2D::CrossProduct(b, ab), FVector2D::DotProduct(b, ab));
          // If the angle is perfectly equal to 0 (lines are parallel), then just use the regular line width (don't divide by 0)
          if (fi != 0)
          {
            // offset of bisector (this is just the line width)
            float d = lineWidth / 2.0;
            // Length of the bisector
            float l = d / sin(fi);
            // Calculate the actual vertices
            targetInnerPoint = targetPos + (l * b);
            targetOuterPoint = targetPos - (l * b);
            UE_LOG(LogCigiSlateVerts, SLATE_VERTS, TEXT("LogCigiCustomVerts: i=%d/%d, target=FANCY from dirs[%d]=(%.3f,%.3f) to -dirs[%d]=(%.3f,%.3f) using fi=%.3f and l=%.3f, created inner=(%.3f,%.3f) and outer=(%.3f,%.3f)"), i, points.Num() - 1, i, dirs[i].X, dirs[i].Y, i + 1, (-dirs[i + 1]).X,
                   (-dirs[i + 1]).Y, fi, l, targetInnerPoint.X, targetInnerPoint.Y, targetOuterPoint.X, targetOuterPoint.Y);
          }
        }
        else
        {
          UE_LOG(LogCigiSlateVerts, SLATE_VERTS, TEXT("LogCigiCustomVerts: i=%d/%d, target=NORMAL"), i, points.Num() - 1);
        }

        AddQuad(color, currInnerPoint, currOuterPoint, targetOuterPoint, targetInnerPoint);
      }

      currPos = targetPos;
      stippleProgress += travelDistance;
      segmentDistance = FVector2D::Distance(currPos, points[i + 1]);
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026