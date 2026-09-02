# Introduction

The CIGI SDK for Unreal is a software plugin designed to enable interoperability between CIGI-compliant host systems and the Unreal game engine. CIGI is a widely used communication interoperability standard in the simulation industry and is supported by a variety of Computer Generated Forces (CGF), Semi-Automated Forces (SAF), and image generation (IG) applications, however CIGI is not typically supported by modern game engines. SimBlocks.io has developed the CIGI SDK for Unreal to allow developers to leverage existing simulation applications and communication interoperability standards when using Unreal Engine to build immersive applications. Note that the CIGI SDK for Unreal primarily provides a software integration where the intention is for the user to combine the SDK with their Unreal assets.

There are two main versions of CIGI in use today (v3.3 and v4.0), and our SDK is compatible with both versions. Version 4.0 has been approved by the Simulation Interoperability Standards Organization (SISO). 

A CIGI-compliant host is required for testing. The SimBlocks.io CIGI Host Emulator is available with this release package with many example scripts to get started. 

# Setting Up the Repository

1. Clone the CIGI-Unreal-Plugin repository  
**It is recommended to clone from the https://github.com/SimBlocks/CIGI repository in order to have the simulationsdk and thirdparty dependencies setup for the CIGI-Unreal-Plugin to build.**

2. Navigate to the CIGI-Unreal-Plugin folder in File Explorer.
   ![UnrealCigi.uproject](https://github.com/SimBlocks/CIGI-Unreal-Plugin/blob/main/screenshots/UnrealCigi.uproject.png)

3. Right click on the UnrealCigi.uproject and select 'Switch Unreal Engine Version'
   ![Switch Unreal Engine Version](https://github.com/SimBlocks/CIGI-Unreal-Plugin/blob/main/screenshots/Select_Unreal_Engine_Version.png)

4. Select your desired Unreal Engine version and hit **OK**.
      
5. Run copy_DLLs.bat.

6. Open UnrealCigi.sln in Visual Studio 2022.

7. Set the configuration to Development Editor and the platform to Win64. 

8. Build the solution.

9. Press the play button to run.

# Using Unreal with the SimBlocks CIGI Host Emulator

1. Now that Unreal is running with the CIGI scene, launch the HostEmulator from Visual Studio.
![Launch Host Eumulator](https://github.com/SimBlocks/CIGI-Unreal-Plugin/blob/main/screenshots/Visual%20Studio%20-%20HostEmulator.png)

2. In the Host Emulator, go to the **Scripts** tab. Select **Unreal Full Tests**. Then select **Entity Control - CreateEntity**. Then click **Run Script**.
![Launch Host Eumulator](https://github.com/SimBlocks/CIGI-Unreal-Plugin/blob/main/screenshots/Host%20Emulator%20-%20CreateEntity.png)

3. You should see a simple scene loaded with an aircraft and landscape.
![Launch Host Eumulator](https://github.com/SimBlocks/CIGI-Unreal-Plugin/blob/main/screenshots/Unreal%20-%20CreateEntity.png)

4. On the Host Emulator Script tab, click the **Stop Script** button.

5. Feel free to test other scripts in the Host Emulator.

# Support

For feature requests and to report issues for this product, please utilize the GitHub Issues page.

For more customized software support or Unreal application development, please reach out to our team at sales@simblocks.io

For general CIGI questions, please contact the CIGI Product Support Group at https://sisostandards.connectedcommunity.org/home

