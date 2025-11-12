# daisyOwnProjects
My own daisy seed based projects


# TO KNOW: 
Environment setup and first build video:
https://github.com/electro-smith/DaisyWiki/wiki/1.-Setting-Up-Your-Development-Environment#1-Install-the-Toolchain (more up to date)
https://youtube.com/watch?v=AbvaTdAyJWk


&NewLine;
<br />
To flash:
- In VSCode, open the command palette: `cmd + P`
- To build examples, upgrade DaisySP or LibDaisy: type `task build_all` and hit `Enter`
- Flash (without STLINK): 
  - Put Daisy into BOOT mode: once Daisy is connected to the computer, press and hold `Boot`, then press and hold `Reset`, then release `Reset`, then release `Boot`.
  - Type `task build_and_program_dfu` and hit `Enter`

&NewLine;
- Flash with STLINK:
  - Connect STLINK to Daisy having the red line on the ribbon cable facing _the opposite_ of the USB port. Center has to have an even number of pins on either side of the connector.
  -  Type `task build_and_program` and hit `Enter`

<br />

Electrosmith Forum: \
https://forum.electro-smith.com

Electrosmith Discord: \
https://forum.electro-smith.com

&NewLine;
Compiler type: arm-none-eabi-g++.exe