# Altirra Change Log

## Version 4.20 [December 29, 2023]:
   ### changes
   * Command Line: Switches are no longer allowed at the end of arguments, e.g. disk.atr/f or /f/nosi. Switches must now be separate arguments.
   * Input: Removed the extraneous hidden Button 1 input on light pen/gun controllers.

   ### features added
   * Additions: Added PHLOAD.COM to trigger peripheral handler loads via type 4 polling.
   * Cartridge: Added Williams 16K cartridge type.
   * Cartridge: Changed existing JRC cart type to JRC6 64K without RAM and added JRC RAMBOX cartridge type.
   * Cartridge: Added MDDOS cartridge type.
   * Cartridge: Added COS 32K cartridge type.
   * Cartridge: Added Pronto cartridge type.
   * Cartridge: CAR types 76, 78-93, and 160 are now supported without advanced configuration overrides.
   * Cassette: Load Data as Audio option now supports raw FSK and PWM data blocks in .cas files.
   * Cassette: BASIC auto-boot can now handle BASIC program tapes that require RUN "C" instead of CLOAD.
   * Command Line: Added /[no]casautobasicboot, /kernelref, /basicref, and /diskemu switches.
   * Debugger: Symbols escaped with '#' prefix can now include @ in their name.
   * Debugger: .vbxe_traceblits command now has a compact mode.
   * Debugger: .vbxe_bl command now has a compact mode and can dump a blit list at an arbitrary address.
   * Debugger: Added explicit option in the disassembly window for whether the current D register state affects symbol lookup for direct page addressing modes.
   * Debugger: bta (set tracepoint on access) can now supply a default trace message.
   * Debugger: Math pack symbols can now be unloaded independently.
   * Debugger: Added static trace PBI (stp) command to auto-trace parallel bus interface firmware.
   * Debugger: Memory window now accepts arbitrary address expressions.
   * Debugger: Ctrl+F in the history view moves focus to search box.
   * Debugger: Added c (compare memory) command.
   * Debugger: Added a menu command to export the debugger help from the .help command in HTML form.
   * Debugger: @hwwritereg(addr) can be used to read back write-only system hardware registers.
   * Debugger: The debugger now additionally tries to resolve source file references against the module binary path and the paths of any open source windows.
   * Debugger: Source windows now have context menu options to reopen the file externally.
   * Debugger: u (unassemble) now has -m option to change CPU mode used for disassembly.
   * Devices: 1030 now does actual pulse/tone dialing.
   * Devices: 1030 relay sound is now heard when phone is placed on-hook.
   * Devices: Added SIDE 3.1 (JED 1.4) support.
   * Devices: Added CPS SuperSALT Test Assembly support.
   * Devices: MidiMate now exposes a serial port for custom MIDI interfacing.
   * Devices: File writer device can now also be used with serial ports.
   * Devices: Added networked serial port device.
   * Devices: Added copy/paste support to devices UI.
   * Devices: Added shortcut command to rescan a virtual FAT16/FAT32 disk.
   * Devices: Added 835, full 835, and full 1030 hardware emulation.
   * Devices: Added SIO serial adapter device to bridge serial devices directly to the emulated SIO bus without requiring motor control activation.
   * Display: Added support for main display pan and zoom.
   * Disk: Improved timing compensation when burst I/O mode is enabled.
   * Input: Added support for Chalk Board PowerPad controller.
   * Input: Added support for Stack Lightpen controller.
   * Input: Mouse wheel can now be bound as an input.
   * Input: Added an option to show pointer location when driving a tablet, pad, or light pen/gun with a non-mouse input.
   * Input: Pad input area can now be moved and resized via Configure HUD.
   * Network: Raised max MSS in emulation TCP stack from 512 to 1460.
   * Network: Emulation gateway now supports ICMP echo requests (only to the gateway; not forwarded).
   * Network: Emulation gateway now has a more distinctive Ethernet address to avoid conflicts when bridging through a VXLAN tunnel.
   * PerfAnalyzer: Added support for saving and loading traces.
   * PerfAnalyzer: Added trace channel for disk sectors passing under the drive head.
   * PerfAnalyzer: Added approximated import for Atari800WinPLus 4.0 monitor execution traces (TRON/TROFF).
   * Save States: Devices can now opt-in to save state support.
   * Save States: R-Time 8, XEP80, Black Box, MIO, Ultimate1MB, SIDE/2/3, and MyIDE[-II] are now included in save states.
   * Save States: CPU mode, 65C816 high memory, and Axlon memory are now included in save states.
   * System: Added 1400XL and 1450XLD emulation support (currently only minimal V:/T: support).
   * UI: Added /registeruserfileassocs, /registersysfileassocs, and /unregisterfileassocs for silent registration/unregistration of file type associations.
   * UI: Added support for screen readers to read out text from the emulation screen (opt-in).
   * VFS: Added support for reading from .zip files that use Enhanced Deflate compression.
   * VFS: Add special:// protocol to access specific images.

   [bugs fixed]
   * AltirraOS: Fixed incorrect MEMLO adjustment when loading peripheral handlers through CIO via type 4 poll.
   * AltirraOS: Force C=1 on exit from CIO for compatibility (undocumented behavior).
   * AltirraOS: Shift+Help and Ctrl+Help are now reported properly in HELPFG.
   * AltirraOS: KEYREP and KRPDEL are now supported properly on the 65C816 build.
   * AltirraOS: Improved compatibility of NOCLIK handling.
   * AltirraOS: Fixed K: not applying inverse video to vertical bar ($7C) character.
   * AltirraOS: Fixed timing issue in SETVBV.
   * AltirraOS: S: now properly overrides the no-clear flag for GR.0.
   * AltirraOS: FASC no longer clears first byte of FR0.
   * AltirraOS: SIOV now resets BRKKEY when returning Break key pressed condition.
   * Audio: Fix downmix setting not saving.
   * Cartridge: Phoenix and Blizzard cartridge types now also react to CCTL reads as well as writes.
   * Cartridge: SIC! 128K and 256K images now retain their original type rather than being converted to 512K on load.
   * Cassette: Fixed FLAC decoder crashing on some files on some CPUs.
   * Cassette: BASIC auto-boot detection now handles BASIC programs saved with the rev. B BASIC bug.
   * Cassette: SIO data in line is now released when motor stops.
   * Command line: Address specified in /soundboard switch was always overridden to default.
   * CPU: 65C816 PLB and PLD instructions were not setting NZ flags.
   * CPU: 65C816 PHP instruction could write bits 4/5 as zero after re-entering emulation mode from native mode with MX bits cleared.
   * CPU: Improved accuracy of NMI timing when NMIs occur across a write to WSYNC.
   * Custom Devices: Programmatic memory layer changes no longer bypass auto-PBI.
   * Debugger: Zero page symbols were not being loaded from CC65 symbols.
   * Debugger: PEI (dp) instruction no longer shows indirect data in disassembly.
   * Debugger: Symbol lookup is now suppressed for (dp), (dp,X), and (dp),Y addressing modes with D != 0 like with other direct page addressing modes.
   * Debugger: Fixed Step Over command not working above bank 0.
   * Debugger: Disassembler now does basic block detection in 8048 mode.
   * Debugger: Fixed .diskdumpsec command not working on virtual disks.
   * Debugger: Fixed a crash when clearing breakpoints from debugger symbol based assert/trace directives.
   * Debugger: Fixed panes that displayed the context menu at the wrong place when activated from the keyboard.
   * Debugger: Debug display wasn't decoding IR mode 8 properly.
   * Debugger: Improved visibility of keywords in source views in dark theme.
   * Devices: R-Time 8 is now reset on a cold reset.
   * Devices: SIDE 2 now returns floating bus data instead of $FF when reading from deselected CF card.
   * Devices: Fixed sporadic initial interrupt state issue with MIO.
   * Devices: Fixed some stack wrapping bugs with Veronica's 65C816 CPU.
   * Devices: Fixed "use long path names" setting not saving properly on the H: device.
   * Devices: H: no longer returns Y=$03 for imminent EOF when reading directories.
   * Devices: MIDI output device from MidiMate is now reset on cold reset.
   * Devices: Added missing emulation of IDE+2 CF detect in $D1FA bit 6.
   * Devices: Fixed virtual FAT32 device limiting included files to FAT16 cluster limit.
   * Devices: Fixed 8048 ADDC,@Rn and MOVP3 A,@A instructions and TF emulation.
   * Devices: Fixed broken ATI0/1 commands for SX212.
   * Disk: Fixed XF551 full drive emulation sometimes not handling write protect state properly.
   * Disk: Reinterleave command wasn't marking the disk as modified.
   * Disk: Fixed crash when attempting to convert a disk using an unknown or unsupported filesystem.
   * Disk: Improved Happy 1050 memory map to closer match hardware, including contended regions.
   * Disk: Add workaround for ATX disk images that have extra zero padding words at the end.
   * Disk: Fixed 810, 1050, and XF551 based full drive emulators not consistently acting on write protect status.
   * Disk: Improved accuracy when reading and writing sectors with missing data fields.
   * Disk: Fixed rotational position being updated while motor is off.
   * Disk: Fixed 810 post-read delay being used for 1050 emulation in single density.
   * Disk: Improved timing precision for full disk drive emulators.
   * Disk: Sectors are now ignored on mixed-density disks when not using the current density.
   * Display: Fixed inverted field polarity detection in interlace mode (since 3.90) and improved auto-flip behavior.
   * Display: Removed horizontal offset in PAL high artifacting.
   * HLE: Set Y and C=1 in program loader to emulate CIO results when invoking init/run address.
   * Input: Fixed paddles not responding when driven in absolute mode from a digital input (regression since 4.10).
   * Input: Fix light pen not working in port 4.
   * MMU: Extended RAM priority is now adjusted when U1MB is enabled to emulate difference in VBXE MEMAC behavior.
   * Network: Fixed emulation TCP stack not sending final FIN packet to emunet after graceful shutdown from remote host.
   * Network: Emulation dhcpd no longer reports a gateway to DHCP clients if routing is disabled (NAT off).
   * Network: Fixed VXLAN tunnel code sending two extraneous bytes with each Ethernet frame.
   * PerfAnalyzer: Fixed keyboard shortcuts not working in the perf analyzer's history view.
   * PerfAnalyzer: Snapshot frame timing at end of session so VBI/DLIs don't get confused when emulation is restarted after the trace ends.
   * PerfAnalyzer: Fixed crash in tape code after stopping trace with a tape mounted.
   * POKEY: Improved accuracy of truncated/restarted POTGO scans.
   * POKEY: Fixed stop bits not playing properly out audio channels 1/2 in two-tone mode.
   * Recorder: Fixed bug with AVI writer not marking all uncompressed frames as key frames.
   * Recorder: Fixed incorrect colors when recording WMV or MP4 video without scaling (ARM64 only).
   * Save States: Fixed POKEY having keys stuck down after loading a save state.
   * Save States: Fixed last 64K of extended memory not being saved properly in 128K and 320K memory modes.
   * UI: Fix crash when using Calibrate Screen command without any hardware graphics acceleration.
   * UI: Ensure that undocked tool windows are on-screen when restoring window layout.
   * UI: Added workaround for Windows 10/11 touch keyboard sending the wrong extended key state for movement keys.
   * UI: Fixed Input Setup controller diagrams displaying blank in some configurations.
   * VBXE: Fixed incorrect blitter cycle speed for $00 bytes transformed by AND/XOR mask.
   * VBXE: Blitter collision detection now returns the first hit instead of the last hit.
   * VBXE: Fixed pixel shadows when using HR blit mode (mode 6).
   * VBXE: Fix false P/M priorities on right half of screen in core 1.26 in lines that have no P/M graphics.

## Version 4.10 [January 1, 2023]
   ### changes
   * Display: Direct3D 11 and borderless mode are now enabled by default for new installs.
   * Input: Light pens and light guns have been split for 4.10+. Input presets may need to be updated and offsets readjusted.
   * UI: ANTIC/GTIA visualization options have been moved to the Debug menu.

   ### features added
   * Audio: Added option downmixing stereo POKEY output to mono.
   * Audio: Added 800 console speaker acoustics simulation.
   * Cartridge: Added support for CAR types 71-75 (5200 Super Cart, bank 0 init version of MaxFlash 8Mbit).
   * Cartridge: Added support for the XE Multicart.
   * Cartridge: Added support for SIC+.
   * Cassette: Added config variable to manually tweak balance between mark/space bit detectors.
   * Cassette: Added support for PWM encoded turbo data in CAS files.
   * Cassette: Added support for SIO data out controlled turbo (Turbo Blizzard).
   * Custom Devices: The priority of memory layers can now be customized.
   * Debugger: Memory locations can now be edited in the Memory pane.
   * Debugger: Memory pane now has an option to display 8bpp graphics.
   * Debugger: Randomize Memory on EXE Load option now logs when randomization occurs and uses the global randomization seed.
   * Debugger: .loadobj and .loadmem now take VFS paths.
   * Debugger: Added .error command to decode various error codes.
   * Debugger: Added Breakpoints pane and Ctrl+B as a shortcut for a new breakpoint.
   * Debugger: Added initial support for DWARF5 debugging information from LLVM-MOS .elf files.
   * Debugger: Disassembly window now supports address expressions and mixed source/disassembly view.
   * Devices: Improved selection behavior when adding or removing devices.
   * Devices: NVRAM can now be isolated per profile and follows the temporary profile state.
   * Devices: Added SimCovox support.
   * Devices: Added shortcut command to temporarily mount VHD images in Windows to access FAT32 partitions.
   * Devices: APT partitions on hard disks can now be opened in Disk Explorer.
   * Disk: Added emulation of buffered track reads for Happy 1050 and Speedy 1050 devices in standard emulation mode.
   * Disk: 810/1050 full drive emulations now warn on attempts to use double-sided disks that can't be read in those drives.
   * Display: SDR/HDR balance in HDR mode can now use the system-wide SDR intensity setting.
   * Display: Added support for DXGI 1.3 seamless custom refresh rate support on integrated display panels.
   * Display: Reduced buffered frame latency in display paths.
   * Display: Added adaptive speed option to lock to refresh rate for reduced latency.
   * Firmware: Added detection of several XF551 firmware ROMs.
   * HDevice: Added full long filename mode.
   * HLE: Program loader now simulates SDFS disk in Disk Boot mode to auto-trigger EXE load under SDX.
   * Input: Extended immediate pots option to allow support immediate light pen/gun inputs.
   * Input: Added interactive tool to recalibrate light pen/gun offset.
   * Input: Improved accuracy of light pen/gun trigger pulse timing.
   * Input: Added an option for light pen/gun noise.
   * PerfAnalyzer: Added support for exporting to Chrome tracing format.
   * Tape Editor: Bit position is now marked purple if a single bit flip is identified that would fix a bad checksum in a standard block.
   * UI: The Check for Updates dialog now has a link to the internal RSS feed used to deliver the new update info.
   * UI: Added support for copying and pasting characters from the interactional character set.
   * UI: Keyboard shortcut dialog now has better support for binding Alt+keys and is more keyboard accessible.
   * UI: Added dark mode support for checkboxes and radio buttons.
   * UI: Alt+click decode can now decode MyDOS error messages.
   * UI: Paste can now paste visible control characters from Unicode.
   * UI: Improved clarity of screenshots captured with true aspect ratio by changing image filter.

   ### bugs fixed
   * Additions: Fixed BRK instruction in LOADEXE.COM.
   * AltirraOS: Improved compatibility of TIMFLG usage and initial register values on tape boot.
   * Cartridge: Fixed banking issues with SIC! during flashing and when loading save states.
   * Cassette: Accelerated C: loads now handle block reads with timeouts longer than 30 seconds.
   * Cassette: Accelerated C: loads now set TIMFLG.
   * Cassette: Fixed FLAC decoder errors with some files.
   * Custom Devices: Blocked ++/-- increment operators to avoid confusion (they were parsed as pairs of unary operators, which was usually not intended).
   * Debugger: Fixed memory window horizontal scrollbar not updating after some changes.
   * Debugger: Fixed long pauses or broken history listings when toggling the collapse options in the History window.
   * Debugger: Fixed .printf command outputting a garbled character with %d/i format.
   * Debugger: Improved load speed of source windows.
   * Debugger: Fixed symbol-related crash when opening source windows with deferred symbol loads.
   * Debugger: Fixed Step Out/Into commands not using source mode in an undocked source window.
   * Debugger: Fast boot hook now marks memory as preset to avoid tripping false uninitialized memory errors when heat map trapping is enabled.
   * Debugger: Source window was keeping stale source line data when reloading symbols.
   * Debugger: Fixed ##BANK annotations in MADS listings not being parsed properly with decimal bank numbers.
   * Debugger: Fixed bug where memory access breakpoints would stop working after clearing other access breakpoints at the same address.
   * Devices: Fixed SD card multiple-block write command.
   * Devices: Fixed SD card state not being reset on SD power toggle.
   * Devices: Fixed bug with virtual FAT16/FAT32 hard drives returning bad sector data when re-reading final partial data sectors of a file.
   * Disk: Fixed too short timeout for Record Not Found (RNF) errors.
   * Disk: Improved sound of recalibrations in 810-based standard emulation modes.
   * Disk: Retuned receive rates for Happy 810 and Happy 1050 standard emulation modes.
   * Disk: Fixed idle motor-off timeout occurring too quickly after long commands with retries.
   * Disk: XF551 full emulator now supports IN A,BUS instruction used by some modified firmware.
   * Disk Explorer: Fix incorrect Unicode mappings for control characters in viewer.
   * Display: Reduced frame jitter in D3D11 windowed mode on Windows 8.1+.
   * Display: Fixed monochrome modes with VBXE or high artifacting.
   * Display: Fixed low quality text rendering in D3D11 mode after a change in HDR mode.
   * Display: Fixed crash when forced out of D3D11 exclusive full screen mode by OS due to an overlapping window.
   * HDevice: Fixed append mode not working for files on H6-H9:.
   * Input: Fixed vertical offset on light pen/gun position in PAL.
   * Input: Add Ctrl+Shift+Alt+0 as an alias for Ctrl+Shift+0 to work around Windows stealing this key for a keyboard layout switching shortcut.
   * PerfAnalyzer: Tape channels now show proper jumps when accelerated loads occur.
   * PerfAnalyzer: Fixed NMI showing as IRQ when NMI preempts IRQ.
   * PCLink: Directory enumerations now report if a directory is the root or a subdirectory.
   * PCLink: Improved path compatibility with standard SpartaDOS X path parser.
   * POKEY: Fixed glitches in low linked timer 3 audio output when updating linked timer 3+4 state.
   * POKEY: Improved accuracy of serial port clock timing.
   * POKEY: Serial output bit state is no longer reset by initialization mode.
   * POKEY: Improved accuracy of timer reloads.
   * Profiler: Fixed negative unhalted cycle counts showing up around STA WSYNC instructions.
   * SaveStates: Fixed bug where sprite engine could get desynced after loading a save state.
   * Tape Editor: Fixed draw tool using wrong threshold for bit polarity with waveform display enabled.
   * Tape Editor: Fixed SIO capture option not showing enable state on menu.
   * Tape Editor: Captured accelerated C: loads now show sync bytes.
   * Tape Editor: Block checksum status is more reliably shown during live SIO data capture.
   * Tape Editor: Fixed extraneous bit data being added when saving a new CAS file after editing.
   * UI: Reduced glitches when toggling the debugger or full screen mode.
   * UI: /startuplog can now exclude log channels with '-' and ignores Ctrl+C.
   * UI: Removed delay when switching from very slow speed (1%) to t.
   * UI: Progress dialogs are now suppressed in exclusive full screen mode to avoid popping out of full screen when loading tapes.
   * UI: Fixed non-native file browser not working with UNC paths.
   * UI: Fixed focus glitching when selecting a docked tab with an undocked pane selected.
   * UI: Fixed an issue where Unicode text would sometimes be read out of the clipboard as ANSI text.

## Version 4.01:
   ### bugs fixed
   * Additions: Fixed LOADEXE.XEX using old $FD device SIO code for the program loader instead of $7D.
   * AltirraOS: Fixed math pack compatibility issue with FDIV modifying FLPTR (fixes B-Graph pie chart routine).
   * AltirraOS: Fixed a cursor position checking bug with split screen Gr.0 (fixes the BASIC game House of Usher).
   * AltirraOS: Fixed compatibility issue with KEYDEL handling (fixes cursor movement in BrushupV40).
   * ATBasic: IOCB#7 is now automatically closed on I/O errors to avoid SAVE files being kept open for write.
   * Cassette: Accelerated C: loads now properly leave tape running after block load.
   * Cassette: Fixed FLAC decoder crash with some block sizes.
   * Cassette: Fixed regression with FSK blocks in CAS files being read as turbo pulses instead of FSK pulses.
   * Disk: Retuned XF551 high-speed C/E to data frame delay.
   * Display: Workaround for D3D11 minimum precision crash on Intel Iris Xe 30.0.101.1069 driver.
   * Serial: Pad out 850 relocator to match length of original firmware relocator (fixes compatibility with loaders that hardcode the relocator length).
   * VFS: Fixed wrong file being accessed when using atfs:// paths to access files inside a DOS 2 disk image.

## Version 4.00 [November 13, 2021]
   ### changes
   * Display: Dropped support for DirectDraw and OpenGL.
   * Input: Relative mouse binding and 5200 trak-ball speeds have been adjusted for better uniformity. Existing input maps may need to have their speeds adjusted.
   * Platform: Dropped support for Windows XP/Vista; Windows 7 or higher is now required.
   * UI: Keyboard shortcuts have been reset to default to fix a bug with them being stored in the wrong place and to only store changes from default shortcuts.
   * UI: Default shortcuts for toggling audio channels have been changed from Ctrl+Alt+[1-4] to Alt+Shift+[1-4] to fix a conflict with some AltGr+key bindings in the Spanish keyboard layout.

   ### features added
   * Additions: Replaced ALTXEP8F.SYS fast XEP80 driver with ultra-speed ALTXEP8U.SYS driver with symmetric 31Kbaud communication.
   * Additions: Added ALTXEP8P.SYS and ALTXEPUP.SYS variants with auto-switching to PAL (50Hz) on initialization.
   * Additions: Added XEPVHOLD.COM utility to improve XEP80 video timing compatibility with modern displays.
   * Cassette: Added support for KSO Turbo 2000.
   * Cassette: Added new level-based turbo decoding filter.
   * Cassette: Added FLAC decoding support.
   * Cassette: Added feature to hold data line across VBI to increase reliability of OS C: reads.
   * Custom Devices: Scripting support added for break, while, do/while, and forward declarations.
   * Custom Devices: Added support for creating alternate video outputs and PBI IRQs.
   * Debugger: On-screen watches can now update continuously as well as per-frame.
   * Debugger: Watch expression (wx) now has options for hex formatting.
   * Debugger: Extended cartridge bank debugging support to MaxFlash 1Mbit and SDX 64K/128K cartridge types.
   * Debugger: Memory window now supports scrolling, variable width, and graphics decoding.
   * Debugger: Improved speed of console output window.
   * Debugger: Added .fpaccel command to selectively toggle math pack acceleration routines.
   * Debugger: Effective addresses are now shown in history in 65C816 mode.
   * Debugger: Added .pagesums command.
   * Debugger: lft -u option enables logging with microsecond timestamps, and -r enables raw timestamps.
   * Debugger: "Auto-reload ROMs on cold reset" setting is now saved.
   * Debugger: Go frame (gf) command now has a quiet (-q) option.
   * Debugger: 65C816 M/X mode handling can now be configured in the disassembly window.
   * Debugger: .diskdumpsec now has invert (-i) option.
   * Debugger: Added debug link support to receive SDX symbol information from ATDEBUGX.SYS driver.
   * Debugger: Added 'dbx' command to evaluate an expression over a range.
   * Debugger: Added verifier options for detecting stack wrapping and stack in page zero.
   * Devices: Added Percom AT88-SPD emulation.
   * Devices: Percom AT-88 emulation now allows 1791/1795 FDC selection.
   * Devices: Added SIDE 3 emulation.
   * Devices: Added virtual hard disk device that translates a read-only host directory to a FAT16, FAT32, or SDFS partition image.
   * Devices: Added temporary write filter hard disk device.
   * Devices: Added preview SoundBoard 2.0 emulation.
   * Devices: Added 1090 80 Column Video Card and Bit 3 Full-View 80 emulation.
   * Devices: Added parallel port buses to all devices that support printer output and file writer device to sink printer output to a file.
   * Disk: Disk rotational positions are now randomized on periperal reset.
   * Disk: ATR loader now matches 77-track disk geometries for 8" disks.
   * Disk: Added support for Happy 810 Autospeed mod.
   * Disk: Percom RFD-40S1 double-clock mode is now implemented.
   * Disk: Added option in Disk Drives dialog to show location of mounted disk images in File Explorer.
   * Disk: Added full emulation for the Atari 815 disk drive (read only).
   * Disk: Added support for reading from and writing to Indus GT CP/M 2.2 filesystems.
   * Disk Explorer: Added options to import/export files with text (line ending) translation.
   * Display: New Palette Solver in Adjust Colors dialog to derive color parameters from a palette file or photograph of a palette image.
   * Display: Added white monochrome mode.
   * Display: Added option to constrain mouse pointer to active display in full-screen mode.
   * Display: Added high dynamic range (HDR) display support (requires Windows 10 / DXGI 1.6).
   * Firmware: Warn if ROM image being added is blank.
   * Firmware: Added autodetection of 1200XL rev. 11 OS and 1050 rev. H.
   * HDevice: Lifted 16MB file size limit for untranslated access (H1-H4:).
   * IDE: VHD differencing mode images can now be mounted.
   * Input: Added support for XInput 1.4.
   * Input: Added an option to use the Raw Input API (Configure System > Input).
   * Input: Added an option for immediate potentiometer updates to reduce paddle and 5200 controller latency.
   * Input: Mouse, trackball, and 5200 controllers now accept mouse delta bindings to Left/Right/Up/Down, and paddle controllers accept digital bindings to Left/Right.
   * Input: Added preset template generator for input maps to make it easier to set up less common controller setups.
   * Input: Ctrl/Shift are now optionally blocked from virtual key mappings when used by input maps with the 'share' option off. (They are still processed for character mappings due to OS limitations.)
   * PerfAnalyzer: Added trace channel for SIO command state.
   * Platform: Added option to poll virtual disk and H:/PCLink sources for systems that have broken file change notifications.
   * POKEY: Improved audio filters for better performance and aliasing rejection.
   * Simulator: Added centralized randomization control under boot options.
   * Simulator: Throttling code updated with waitable timers for better efficiency.
   * UI: Dark mode improved with reskinned buttons.
   * UI: Added option to auto-hide menu in windowed mode.
   * UI: Added advanced configuration dialog to expose hidden, unsupported parameters.
   * UI: Performance analyzer now supports dark mode.
   * UI: Added /reset to load last profile without the device tree.
   * UI: Right-clicking on a menu item now provides a direct path to open Keyboard Shortcuts to bind a key to that menu item.
   * UI: Added /tapepos to pre-seek tape to specified position.
   * XEP80: Improved accuracy of XEP80 bit and byte timing based on measurements on real hardware.
   * XEP80: Added NS405 FIFO timing emulation.

   ### bugs fixed
   * 5200: Rewrote noise implementation again to work around games with buggy joystick position filters (Galaxian) and to avoid drift for relative bindings.
   * Additions: BOOT850 no longer leaves CRITIC set on failure.
   * ANTIC: Fixed unstable horizontal scrolling in IR mode 3.
   * ANTIC: JVB instructions now have the correct replayed height when ending a vertical scroll region.
   * Audio: Changes to the default audio device are now tracked in WASAPI mode.
   * Audio: Improved aliasing rejection of output audio filter.
   * Cartridge: Fixed MegaCart 512K (3) banking disable behavior to only use bit 7.
   * Cassette: Fixed regression with proceed/interrupt turbo modes not working.
   * Cassette: Turbo decoding/filtering settings now apply to the "Analyze tape decoding" command.
   * Cassette: Motor and play/record/pause state is no longer reset when switching tapes.
   * Custom Devices: Fixed script compiler allowing async methods to be called from handler contexts that don't support async operations.
   * Debugger: The Find Source File dialog now strips subdirectories when populating the file filter.
   * Debugger: Fixed heat map not reflecting reads from some read-modify ALU instructions (e.g. ADC zp,X).
   * Debugger: Improved performance of tracepoints that check return values by suppressing UI updates on the hidden breakpoint.
   * Debugger: .disktrack command now works with geometries other than 18 sectors/track.
   * Debugger: .diskreadsec now always uses the virtual sector length for consistency with .diskwritesec.
   * Debugger: Fixed disassembler prescanner using the wrong instruction length for COP n instructions.
   * Debugger: Removed JSR (abs,X) and added RTL and BRL to list of opcodes that trigger a procedure separator.
   * Debugger: Fixed crash in 65C816 mode with access breakpoints with the same page address but different bank address.
   * Debugger: Step Over at disassembly level now handles JSR (abs,X), JSL, and MVN/MVP opcodes.
   * Debugger: Added .vbxe_pal command.
   * Debugger: Fixed disassembly window repeatedly trying to scroll near the top of address space.
   * Devices: Fixed a case where Rapidus could fail to cleanly switch from the 65C816 to the 6502.
   * Devices: Fixed bug where debug reads from SIDE/SIDE2 flash windows could trigger side effects during sector erase operations.
   * Devices: Rapidus fast RAM windows no longer have priority over ROM.
   * Devices: Fixed Rapidus main RAM speed override sometimes being reverted; main RAM is now always at slow speed and extended RAM is also slow speed.
   * Devices: Fixed R-Time 8 state being changed by debug reads in the debugger.
   * Disk: Fixed name mangling on virtual SDFS disks for files and directories with names starting with periods.
   * Disk: Fixed corrupted double-density ATRs being written in some cases from full disk drive emulators.
   * Disk: Fixed bug with final write from full drive emulators not always triggering auto-flush.
   * Disk: Fixed corruption of first byte sent by 810/1050 full drive emulators.
   * Disk: Recalibrated ACK-to-Complete delays for the standard disk emulator for Status and Read PERCOM Block commands based on firmware timings.
   * Disk: Virtual SDFS volumes now use creation timestamps for directories.
   * Disk: Added workaround for 1050 Turbo firmware not detecting density if the drive is powered up with a disk already inserted.
   * Disk: Fixed hang when loading ATX images with extra data pointing to out-of-range sectors.
   * Disk: Fixed XF551 formatting side 2 of disks with incorrect interleave.
   * Disk: Motor off timer adjusted for 1050 timing and to not count down during disk operations.
   * Disk: Accurate sector timing mode now waits for the full length of long sectors in 1050-based emulation modes.
   * Disk: Fixed broken audio output from Indus GT disk drives.
   * Disk: Generic disk emulation profile supports XF551 acceleration again.
   * Disk: Fixed several broken/missing instructions in Z80 emulation.
   * Display: Fixed chroma signal having reversed phase in NTSC high artifacting mode, causing chroma artifacts in mono mode to move in the wrong direction.
   * Display: D3D9 driver now allocates swap chain immediately to force a fallback with useless D3D drivers that report PS3.0 but only have 1MB of VRAM.
   * Display: Fixed intensity scale color setting not affecting saturation in NTSC high artifacting mode.
   * Display: Fixed issues in the high artifacting chroma filters causing some excessive banding.
   * HDevice: Fixed burst writes not returning error status correctly.
   * HLE: CRITIC is no longer zeroed from PBI-based SIO acceleration to fix key repeat issues with some versions of SpartaDOS X.
   * IDE: Fixed Rev. S not supporting some Rev. D registers and being detected as Rev. C.
   * Input: Fixed arrow keys getting stuck down in emulation when pressing Alt+arrow and then releasing Alt first, with arrow keys bound to a controller.
   * Input: Quick maps are now always cycled in sorted name order.
   * Input: Fixed controller triggers getting stuck down in some cases when detaching light gun controllers.
   * Input: Fixed flipped horizontal relative movements for tablets.
   * Input: Trigger targets are now collapsed properly so that multiple mappings to the same target cooperate, e.g. Left -> Numpad7, Up -> Numpad7, Up -> Numpad8, with both keys being pressed and released in any order.
   * Input: Direct keyboard layout had incorrect mappings for Ctrl/Shift+Space.
   * PCLink: Fixed reversed directory sort order.
   * PCLink: Now uses last write timestamp for files instead of creation timestamp.
   * PerfAnalyzer: Fixed painting bug with ANTIC DL event channel at some zoom levels and scroll positions.
   * PerfAnalyzer: Fixed timing error for bytes sent to the computer from a full disk drive emulator, which were shown starting at their actual end times.
   * POKEY: Channel mixing updated to use more representative per-channel non-linear steps.
   * POKEY: Fixed channels occasionally having their timing disrupted when toggling timer IRQs.
   * POKEY: Fixed extra audio pulse when timer 1/2 underflow coincides with two-tone timer reset.
   * POKEY: Fixed keys not being re-reported in raw non-full scan mode when enabling debounce with keyboard scan already enabled.
   * UI: Fixed mouse wheel scrolling when the system wheel scroll setting is set to pages instead of lines.
   * UI: Profile editor no longer allows parenting a profile to itself or its children.
   * UI: Settings load/save code now detects profile loops due to the profile editor bug and forces a reparent to the global profile so settings saving still works.
   * UI: ARM64 emulation dialog no longer allows saving "OK" response.
   * UI: Added 5 second timeout when notifying an existing instance in single instance mode to prevent processes piling up if the instance is hung.
   * UI: Full-screen file browser scrolls back to top when changing folders.
   * VBXE: Improved GTIA state syncing when toggling VBXE on the fly.
   * XEP80: Scrolling now always removes one physical line, not a logical line as E: does.
   * XEP80: Fixed an occasional issue with indeterminate state after cold reset.
   * XEP80: Printing a char at the right margin now only inserts a line when EOL is overwritten.
   * XEP80: Delete line can now slice a multi-line logical line like the real XEP80 does.
   * XEP80: Fill EOL ($1C6) command was using the wrong fill byte.

## Version 3.91: [November 11, 2021]
   ### bugs fixed
   * AltirraOS: Adjusted internal variable usage of P: handler for better compatibility with programs that jump into it without opening P: (Monkey Wrench II).
   * AltirraOS: Fixed E: Put Byte routine sometimes returning Y=2 instead of Y=1.
   * AltirraOS: Implemented XL/XE NOCLIK variable.
   * AltirraOS: Fixed minor rounding error in ATN() constant from assembler.
   * AltirraOS: Fixed bugs with E: move left and delete char at column 0 with LMARGN=0.
   * AltirraOS: Added workaround to SIO for devices sending two ACKs instead of ACK+Complete (fixes Indus GT diagnostics zero adjust).
   * Display: Fixed failure to switch to exclusive full-screen mode in D3D9 with bloom enabled.
   * Display: Reduced banding in PAL high artifacting mode.
   * Input: Fixed Shift key interfering with controllers in 5200 mode.
   * SaveStates: Improved reliability of save states.
   * Serial: Fixed hang when dropping modem connection with unread data.
   * Serial: Fixed initial socket data sometimes not being read from modem until first byte is sent.
   * UI: Fixed a crash in dockable pane code with mixed DPI monitors.

## Version 3.90: [June 14, 2020]
   ### features added
   * AltirraOS: The boot screen in the XL/XE version now continues boot on disk insertion rather than forcing a reboot.
   * AltirraOS: Extended memory scan limit for 800 version to $D000 to support 52K expanded configurations.
   * Audio: Individual channels on the secondary POKEY can now be muted.
   * Cartridge: Added JRC 64K + RAM cartridge type.
   * Cassette: Optimized cassette tape emulation routines.
   * Cassette: Added option for high-pass prefiltering before the turbo tape decoder.
   * Debugger: Improved loop detector in history view to more precisely capture loop.
   * Debugger: Added @tapepos pseudovariable.
   * Debugger: Specifying ? for a path argument now opens a file dialog to select the path.
   * Debugger: The disassembly window now has support for showing procedure dividers, previewing called procedures, and jumping to call targets.
   * Debugger: Disassembly and History views now save their disassembly settings.
   * Debugger: Added option to disable automatic system symbols.
   * Debugger: Alt+Shift+click on the display jumps to History at the corresponding beam position.
   * Debugger: Verifier supports detecting access to non-canonical hardware addresses.
   * Debugger: .caslogdata command superceded by updated CASDATA/CASDATA2 logging channels.
   * Devices: XEP80 can now be switched to different controller ports.
   * Disk: Added full emulation for 810 Turbo, Amdek AMDC-I/II, and Percom AT-88 disk drives.
   * Disk: A warning is now displayed when attempting to insert a disk into a drive that doesn't support it (e.g. double-density disk into an 810).
   * Disk: Detailed errors are displayed when a disk image write fails on the host and is remounted virtual read/write.
   * Disk Explorer: Added Import File and Export File commands so that drag and drop is not needed to copy files.
   * Display: Added option for frame blending in linear color space for better accuracy.
   * Display: Color settings can be saved and loaded from external files.
   * Display: Added new default preset for PAL.
   * Display: The Hue Start slider in color settings now has the same I-Q angle definition in PAL quirks mode and works in PAL high artifacting mode.
   * Firmware: Add Firmware now detects OS ROM images even if the specific image is not recognized.
   * Firmware: Added detection for more MIO firmware images and support for auto-trimming padded 16K images down to 8K.
   * Input: The keyboard layout editor now warns if the key mapping being added conflicts with a keyboard shortcut.
   * Recorder: Added options for pixel aspect ratio and frame scaling.
   * Recorder: Added support for recording to WMV and H.264 through Media Foundation on Windows 7 and later.
   * SaveStates: Save state format rewritten to v2 (*.atstate2), now based on JSON within a .zip file for better accessibility and versioning.
   * SaveStates: CPU state can now be saved mid-instruction.
   * Simulator: Reordered frame wait and device poll timing around VBLANK to reduce input and output latency.
   * UI: Added experimental dark theme support.
   * UI: Added /reset command-line switch to selectively reset settings.
   * UI: Audio monitor now shows modified frequencies for two-tone mode, indicators for two-tone mode and asynchronous receive mode, and better shows ultrasonic effects.
   * UI: Added audio scope for viewing raw POKEY output.
   * UI: Added options for controlling mouse pointer visibility.
   * UI: Audio monitor positioning can now be customized.
   * UI: Audio monitor dims channels that have been toggled off.
   * Video: Added PERITEL adapter and monochrome monitor emulation.

   ### bugs fixed
   * AltirraOS: Audio configuration is reset more often on tape reads to produce familiar sounds.
   * AltirraOS: Disk boot can now occur after cassette boot and right cartridge non-boot.
   * AltirraOS: OLDADR usage adjusted for compatibility with SDX QUICKED.SYS.
   * AltirraOS: SIO transmit operations can now be interrupted by User Break.
   * AltirraOS: The Display Handler now properly sets the default background color (COLOR4) to $06 when opening a GR.11 screen.
   * AltirraOS: KRPDEL is now set and keyboard repeat rates are NTSC/PAL adjusted on the XL/XE/XEGS version.
   * AltirraOS: Fixed an SIO issue with an out-of-spec short delay from asserting the command line to first command byte.
   * AltirraOS: Fixed disk boot issues if a PBI device or cartridge init routine issued an SIO request with DAUX2 > 0 before the disk boot.
   * AltirraOS: The boot screen now resets the Break key state properly after a boot is interrupted by the Break key.
   * AltirraOS: Improved native mode compatibility of 816 version.
   * ANTIC: RNMI (400/800 System Reset) is now always synchronized to VBI.
   * Audio: Fixed stereo mixing being twice as loud as mono mixing.
   * CPU: Fixed cycle timing for indexed illegal read-modify-write instructions and ARR #imm ($6B).
   * CPU: Fixed extra instruction being run occasionally for a DLI delayed by WSYNC.
   * CPU: 65C816 (dp,X) addressing mode now always wraps in emulation mode when accessing high byte regardless of DP.
   * CPU: 65C816 (dp) addressing mode properly wraps in emulation mode with DP aligned, except for PEI (dp).
   * CPU: 65C816 WDM instruction now properly fetches two bytes.
   * Debugger: .dumpdsm -s option now interprets 6809 instructions.
   * Debugger: Line number information is now updated properly when using cartridge bank mapping in MADS listings.
   * Debugger: Disassembly window now accepts cartridge bank addresses.
   * Debugger: Fixed broken option to auto-load kernel ROM symbols.
   * Debugger: Reduced frequency of stale memory in the disassembly window.
   * Debugger: Fixed incorrect call nesting in history window in Z80 mode.
   * Debugger: Fixed some disassembly errors in 6809 mode.
   * Debugger: Input byte (ib) now correctly issues read cycles with side effects in the main CPU address space.
   * Devices: Fixed crash on ANTIC reading from SoundBoard hardware registers.
   * Devices: 65C816 emulation fixes for Veronica.
   * Disk: Fixed some illegal indexed RMW instructions taking too few cycles for 6502 coprocessors in full disk emulators.
   * Disk: Full drive emulation FDC now allows Write Track commands to proceed immediately when the index signal is already active.
   * Disk: Improved accuracy of RIOT interval timer.
   * Disk: Corrected FDC head load and initial DRQ timings for Write Track command.
   * Disk: Implemented FDC Write Track quirk for back-to-back CRC ($F7) tokens.
   * Disk: Fixed some 6809 coprocessor emulation bugs.
   * Disk: Corrected side 2 sector mappings for ATR8000, Percom, and XF551 full drive emulators.
   * Disk: Fixed case where drive timing could be disrupted after hitting a drive coprocessor breakpoint.
   * Disk: Seek sounds no longer overlap when disk access is accelerated.
   * Disk: Fixed crash when ejecting disk at specific point during disk read command.
   * Disk: FD1771 FDC now properly times out in two revolutions instead of five in 810 full drive mode.
   * Disk: Fixed bogus FDC interrupts from Force Interrupt commands.
   * Disk: Indus GT ignores ready status to match hardware.
   * Disk Explorer: Fixed a filename validation bug with DOS 2 disks where duplicate files could be written if the original filename had extra characters after the extension.
   * Disk Explorer: Modifying a mounted disk image now forces a disk change to flush caches on emulated full drives that do track buffering.
   * Display: Fixed an issue with color profiles having the wrong color matching setting when initially established.
   * Display: Fixed crash when pasting into enhanced text display in CIO mode.
   * Display: Typing in enhanced text display (CIO mode) now clears the attract mode counter.
   * Firmware: Fall back to next available firmware if default firmware for a type is removed.
   * Input: Character mappings in custom keyboard layouts didn't work for non-ASCII extended characters.
   * Input: Changes to custom keyboard layouts didn't take effect immediately.
   * PerfAnalyzer: Joystick state (PIA port A) is now captured in traces.
   * POKEY: Improved accuracy of two-tone mode timing at 1.79MHz.
   * Recorder: Fixed SAP type R initial silence detector checking AUDFx registers instead of AUDCx registers.
   * SaveStates: POKEY serial input state is now saved.
   * SCSI: Fixed BSY being asserted in selection phase even for non-existent devices.
   * Simulator: Improved accuracy of power-up values for POKEY and GTIA registers.
   * Simulator: Display message via placeholder ROM if kernel ROM file could not be loaded.
   * VBXE: Fixed a crash when toggling shared memory option.
   * VBXE: Fixed double correction of display with gamma or color correction options enabled.
   * VFS: Fixed zip/gz paths with non-ASCII characters not being encoded properly and getting dropped on restart.

## Version 3.20: [June 23, 2019]
   ### changes
   * Debugging symbols and debugger scripts are no longer automatically loaded by default unless the debugger is active. This can be changed in the Debugger section of Configure System.
   * AltirraOS is now versioned independently of the main emulator. See changelog in export ROM set output for details.
   * The direct read filter for cassette tapes has been lowered in bandwidth to reduce load failures on FSK-encoded glitches. The filter bandwidth can now be adjusted in Configure System, Cassette.

   ### features added
   * Additions: Colormap now supports CTIA systems.
   * AltirraOS: Minor improvements to text I/O performance.
   * Cartridge: Flash memory type can now be changed for Maxflash 8mb cartridges.
   * Cassette: Added option for detecting and automatically switching BASIC when booting tapes based on whether the tape starts with a BASIC or binary program.
   * Debugger: #comments can now be used in .atdbg files.
   * Debugger: @(...) within an expression causes that subexpression to be evaluated immediately.
   * Debugger: .diskdumpsec now dumps phantom sectors for a virtual sector.
   * Debugger: .disktrack now has an option for drive number.
   * Debugger: Added support for bank-specific addresses in MaxFlash cartridges using t:$XX'YYYY syntax.
   * Debugger: Added options for controlling auto-load of symbols and debugger scripts.
   * Debugger: Added menu option to list source files associated with line debugging information.
   * Devices: Added emulation of APE Warp+ OS 32-in-1 device.
   * Disk: Added support for physical sector size chunk in ATX images.
   * Disk: Fixed XF551 standard emulation mode to force SD format with command $21 when the current mode is enhanced density.
   * Display: Added new overscan mode for typical widescreen displays.
   * Display: Added controls for scanline intensity and scaling overall display intensity.
   * Display: Optimized standard NTSC artifacting algorithm.
   * Display: Added option to hide all on-screen indicator overlays.
   * Display: Added color preset for NTSC 800 computer.
   * Display: Added option to accelerate some screen effects using shaders when Direct3D 11 is enabled.
   * Firmware: Version numbers are now displayed for the internal OS and BASIC ROM images.
   * HLE: Program loader now detects and throws an error on an attempt to load an MS-DOS or Windows program into the emulator.
   * Input: Added Arrow Keys -> Paddle A preset.
   * Input: Digital sources can now be bound to the Paddle Knob input on paddle controllers to use the paddle lines as switches.
   * U1MB: Stereo POKEY and Covox control is automatically enabled when Ultimate1MB and stereo/Covox are both enabled.
   * UI: Added a GUI option in settings for switching between portable and registry mode.
   * UI: Added visual drop targets for different drag and drop modes, formerly only available via right-drag-drop.
   * UI: Added overview and recommendation pages to Configure System.
   * UI: Escaping is now supported for text copy and paste.
   * UI: Alt+click message help has been extended to also decode common POKE and PEEK addresses.

   ### bugs fixed
   * AltirraOS: FDIV no longer returns 0 for 0/0.
   * AltirraOS: AFP(".") properly returns an error instead of 0.
   * AltirraOS: EXP10() was returning an error instead of underflowing to 0 for some large negative inputs.
   * AltirraOS: Pressing reset on the boot screen now forces BASIC to do a cold boot.
   * AltirraOS: Type 3 poll loop is now exited on user break error to fix infinite boot loop with Black Box firmware.
   * AltirraOS: Corrected ICBLLZ/ICBLHZ values after CIO get commands.
   * AltirraOS: Pressing Help now sets HELPFG.
   * ATBasic: Fixed crash when Break is pressed prior to startup banner.
   * Audio: Fixed crash on system with no sound card.
   * Cartridge: Manually inserted BASIC cartridges are now persisted across runs of the emulator.
   * Cartridge: Fixed broken DB 32K mode (CAR type 5).
   * Cartridge: The!Cart 16K banking modes no longer use the secondary bank write protect flag.
   * Cartridge: Fixed The!Cart disable mode sometimes leaving bank windows active.
   * Cartridge: Corrected definition of MegaMax 2M mode (CAR type 61) to support banking on reads.
   * Cartridge: Corrected definition of 2M megacart mode (CAR type 64).
   * Cassette: References to tapes mounted directly as .zip files without a direct reference to a file within the zip are now persisted properly across runs.
   * Debugger: Improved Turbo-Basic XL support in the .basic_vars and .basic_dumpstack commands.
   * Debugger: Fixed listing directives getting executed twice for carts.
   * Debugger: Step Over past an interrupt no longer exits the current scope.
   * Debugger: Mini-assembler no longer emits ROR opcodes as LSR.
   * Debugger: Fix crash in MADS symbol parser with single-character labels.
   * Debugger: .readmem and .writemem now accept quoted paths.
   * Devices: Black Box no longer powers up disabled on startup if exactly DIP switches 1-4 are enabled.
   * Disk: Full disk emulators now report address CRC errors properly ($E7).
   * Disk: Improved accuracy of disk errors from XF551 full drive emulation.
   * Disk: Improved emulation of FDC not ready bit ($80).
   * Disk: Read Address now returns sectors with data CRC errors.
   * Disk: The non-standard sector size option of the 1771 is now partially supported.
   * Disk: Deleted sector marks can now be written by the Write Sector command.
   * Disk: Interrupted Write Sector commands now affect the disk image.
   * Disk: Rotate disk command now works with full disk drive emulation.
   * Disk Explorer: Fixed crash when write flush fails due to another program locking the disk image file.
   * Display: Fixed a high CPU usage issue with enhanced text display when paused.
   * HLE: FP acceleration no longer returns 0 for 0/0.
   * SAP: Lowered SAP type D player from $0800 to $0400 to improve compatibility with files that have a low load address.
   * Input: Toggling Held Keys On Reset back off no longer queues Ctrl+Shift+A for next reset.
   * Input: Fixed Shift key state sometimes being incorrect when Shift was pressed/released while holding another key. The emulated Shift key state is now updated on Shift key changes while other keys are held.
   * Input: The Input State and Console controllers are no longer blocked in 5200 mode.
   * PerfAnalyzer: Fixed sporadic errors in profiling sub-view when profiling short CPU trace ranges.
   * Profiler: Fixed a bug causing the function and basic block modes to misattribute some interrupt time to the parent function/block.
   * Profiler: Fixed broken BASIC line profiling mode.
   * Save States: Fixed save state errors with 65C816 CPU modes.
   * Simulator: Fixed crash when single-stepping prior to computer being turned on with disk drive CPU selected.
   * UI: Fixed CIO acceleration options not being bound correctly in new configure system dialog.
   * UI: Disk drives dialog no longer appears hidden the first time it is ever used.
   * UI: Fixed argument parsing error when emulator is launched with argv[0] containing forward slashes.
   * UI: Fixed incorrect partial switch matching on command line, e.g. "/disk1" being interpreted as "/disk 1".
   * VBXE: Overlay width setting %11 now correctly selects narrow width instead of wide width.

## Version 3.10: [August 12, 2018]
   ### changes
   * Default keyboard shortcuts have changed. Reset in Tools > Keyboard Shortcuts to use the new bindings.
   * Modems now default to handshaking disabled rather than RTS/CTS due to differences in default RTS state for R: devices. Use AT&K0 to disable handshaking and AT&K3 to use RTS/CTS.

   ### features added
   * AltirraOS: Added 65C816 native mode capable version.
   * Audio: Added support for XAudio2 and WASAPI sound APIs.
   * Cartridge: Added support for .CAR types 60, 67-70 (Blizzard 32K, XEGS 64K alternate, Atrax 128K raw ordered, aDawliah 32K/64K).
   * Cartridge: Added show-all override to cartridge mapper dialog.
   * Debugger: Added FDCWTDATA logging channel to dump the raw track stream for Write Track commands.
   * Debugger: Added several new verifier modes.
   * Debugger: .basic_rebuildvnt now has a -t option for TurboBasic XL support.
   * Debugger: Added extended memory bank tracking support to the history window and breakpoint systems.
   * Debugger: Add custom symbol (ya) command can now add read-only and write-only symbols.
   * Devices: Added loopback serial device.
   * Devices: UI now shows when a device is missing firmware.
   * Disk: Adjusted default sector interleave patterns to more accurately match real disks.
   * Disk: Added options to UI disk dialog to reinterleave disk images and to convert disk images to different filesystems.
   * Disk: Added option to revert a disk in virtual read/write mode back to last saved contents.
   * Disk: Added full emulation for the I.S. Plate disk drive.
   * Disk Explorer: Added options to relax filename checking and automatically rename conflicting filenames.
   * Display: Added option for color correction from NTSC/PAL colors to sRGB and Adobe RGB color spaces.
   * Display: Added option for auto-switching between NTSC and PAL artifacting modes.
   * Firmware: 400/800 firmware ROM images can now use XL-like 16K layout for $Cxxx extension firmware.
   * HLE: Program loader now has modes to allow the load to be triggered by a type 3 poll or a loader program.
   * HLE: Program loader detects conflicts with and offers to disable internal BASIC.
   * HLE: Fixed main memory not being completed cleared on startup in some cases with Fast Boot enabled.
   * PerfAnalyzer: Profiler view can now be opened on trace sections.
   * PerfAnalyzer: Writes to WSYNC are now considered idle processing.
   * Platforms: Added native ARM64 build.
   * Serial: Added support for emulating modem dialing and handshake sound.
   * Serial: Modem registers S8 and S11 are now supported.
   * Simulator: Added command to cold reset (power-cycle) the computer without power cycling peripherals.
   * UI: Combined many menu options into a combined system configuration dialog.
   * UI: Clicking on non-text area now clears text selection.
   * UI: Added /d3d9 and /d3d11 command-line switches.
   * UI: Reworked command-line help (/?).
   * UI: More system configuration changes now confirm before resetting the computer.
   * UI: Automatic reset for configuration changes is now configurable.
   * UI: The image types automatically unloaded in the Boot Image command can now be configured.
   * UI: The Disk Drives dialog is now modeless.
   * UI: Added overscan option to reserve bottom margin space on screen for the indicators.
   * UI: Enlarged the hit area for dockable pane splitters to make them easier to hit.
   * UI: Main window caption can now be customized.
   * UI: Added option to automatically switch to default profile when launched to start images.
   * VFS: Added support for accessing files within filesystems in disk images (atfs://).

   ### bugs fixed
   * 5200: Default cartridge is now correctly loaded if last mounted cartridge fails to load.
   * Cartridge: Fixed XEGS banking modes in The!Cart emulation.
   * Cassette: Dirty state is now tracked for tapes.
   * Debugger: .dumpdsm command can now be used above bank 0.
   * Debugger: Fixed Y/P register display issues in history with special registers enabled.
   * Debugger: Fixed repaint issue in history window related to removing NEXT node.
   * Debugger: .vectors now reports the native IRQ and emulation COP vectors correctly and no longer reports a bogus native reset vector.
   * Debugger: Fixed invalid effective address being displayed in disassembly for [dp] and [dp],Y addressing modes.
   * Debugger: Suppress operand label decoding for direct page modes in 65C816 mode if D<>0.
   * Debugger: 6502 coprocessors no longer show a 65C816-style 16-bit accumulator in register dumps.
   * Debugger: Fixed power-on delay option clearing pending step operations immediately after cold reset.
   * Devices: Added cooldown timer to browser (B:) device on deny to prevent modal dialog spamming.
   * Devices: Fixed kernel ROM override conflict between Rapidus and U1MB.
   * Devices: Rapidus no longer switches back to 6502 on warm reset.
   * Disk: Fixed crash when attempting to recursively expand .ARC files on disk image with unrecognized filesystem.
   * Disk: Fixed some issues with zero-byte file and subdirectory creation on MyDOS disks.
   * Disk: Fixed occasional filesystem errors when expanding .ARC files on an SDFS disk.
   * Disk: Adjusted DD 512 SDFS boot sector to pass CLX validation.
   * Disk: Attempting to boot an .ARC sourced disk image no longer crashes in the boot process.
   * Disk: DOS 1 format now uses correct VTOC signature.
   * Disk: Fixed directory update bug on DOS2/MyDOS disks when deleting the last file on the last directory sector.
   * Disk Explorer: Creation timestamp is now preserved when dragging files in and out of an SDFS volume.
   * Display: Added workaround for fullscreen mode failing on some versions of VMWare SVGA 3D driver.
   * Display: Gamma correction setting now works in PAL high artifacting mode.
   * Display: ANTIC DMA analysis mode now displays when artifacting is enabled.
   * Display: Fixed bug with PAL high artifacting ignoring odd hires pixels.
   * Display: Artifacting phase and saturation are now hooked up for PAL high artifacting.
   * HLE: Improve tolerance for nonstandard CIO path termination (fixes MultiBASIC DIR command with H:).
   * Input: Fixed startup crash when DirectInput 8 is not available.
   * Input: Fixed "allow Shift key to be detected on cold reset" option not working when disabled and full raw keyboard scan was enabled.
   * MMU: Fixed another Axlon/PORTB memory aliasing issue.
   * POKEY: Improved accuracy of high-pass filter.
   * Serial: Fixed crash in R-Verter emulation if remote connection sent characters before the CONNECT message had been sent.
   * Serial: Fixed slightly inaccurate baud rates at high speeds with 850 full emulation.
   * Serial: Fixed block output mode, two stop bit mode, external input buffer sizing, and other misc bugs in the full 850 handler.
   * Serial: Modem S12 register now controls command sequence guard time.
   * Serial: Socket handling rewritten so that hanging up while a connection is still in progress no longer hangs the emulator until the connection attempt times out.
   * Simulator: Corrected mapping issue for 24K/32K/40K memory modes.
   * U1MB: $D500-D5BF memory is now forced on in config unlocked state.
   * UI: Save states and SAP files now load correctly when dragged from .zip files.
   * VBXE: ANTIC hires pixels now always have PF2/PF3 priority except with XCOLOR=1 on FX1.26 core where enabled pixels have PF1 priority.

## Version 3.00: [December 28, 2017]
   ### changes
   * Devices: Internal device tree storage has changed. Device trees may need to be remade.
   * Input: Added detection and button naming for DualShock 4 controllers.
   * VBXE: VideoBoard XE configuration has been moved from the menu to a device entry.

   ### features added
   * Cassette: Added initial support for turbo tape decoding.
   * Cassette: Mounted tape is now persisted along with other mounted images.
   * Cassette: %-age position is now shown on HUD.
   * Cassette: Improved FSK decoder.
   * Cassette: Added analysis tool to compare raw signal against FSK and turbo decoder outputs.
   * Cassette: Added option to export decoded tape back out as audio.
   * Debugger: History pane now has a more powerful loop detector that can handle larger loops and call/loop nesting.
   * Debugger: History pane can now label each instruction with tape position.
   * Debugger: .tapedata command now accepts position by sample (-s).
   * Debugger: Added bsc command to change condition on a breakpoint.
   * Debugger: Undocumented store and read/modify/write instructions are now shown with write symbols rather than read symbols.
   * Debugger: Extended .pia command output.
   * Debugger: .basic_dumpline now supports reading program text from extended memory.
   * Debugger: ~ (target info) command displays coprocessor types and speeds.
   * Debugger: Added support for per-instruction breakpoints.
   * Debugger: u (unassemble) command now has switches to track 65C816 M/X/E state.
   * Debugger: Memory access breakpoints are now supported in banks $01-FF.
   * Debugger: added bta (set tracepoint on memory access) command.
   * Debugger: db/dw/dsb/dsw/dsd expression operators now accept 24-bit addresses.
   * Debugger: Symbols can now be present above bank 0.
   * Devices: Added browser device (B:).
   * Devices: Added feature to IDEPlus 2.0 to guard against NVRAM corruption on reset during clock reads.
   * Devices: Added XEL-CF adapter emulation.
   * Devices: Added Rapidus Accelerator emulation.
   * Devices: An automatic reboot is now requested before adding or removing devices that practically require one.
   * Disk: ATX images are now saved with a unique creator code.
   * Disk: Added support for ATX images with MFM tracks.
   * PCLink: Added option to use creation timestamps passed in commands.
   * SCSI: Fast/slow block storage speed is now implemented on SCSI buses.
   * UI: Added support for per-monitor V2 DPI awareness in Windows 10 Creator's Update.
   * VBXE: Added option to emulate FX1.24 or FX1.26 behavior.
   * VBXE: Overlay collision detection is now implemented.
   * VBXE: Improved blitter timing precision to sub-scanline.

   ### bugs fixed
   * AltirraOS: Fixed polarity of CKEY flag.
   * AltirraOS: Enforce A=0 on exit from SIOV.
   * ATBasic: Added compatibility workaround for programs that use locations 183 and 184 to read the current DATA line.
   * Cartridge: Fixed error when loading untagged cartridge from drag-and-drop stream source.
   * Cheats: Fixed sorting issues in active list.
   * CPU: Fixed 65C816 being reverted to emulation mode when changing CPU speed dynamically.
   * CPU: Interrupts in 65C816 emulation mode now force K=0.
   * CPU: Fix incorrect extra cycle for opcode $04 (NOP zp).
   * Debugger: Disassembly window now scrolls properly above bank 0.
   * Debugger: Improved breakpoint and stepping behavior with coprocessors.
   * Debugger: Fixed incorrect execution history timings for Percom disk drives.
   * Debugger: Fixed crash when toggling breakpoints through UI on top of hidden break/tracepoints from loaded symbols.
   * Debugger: Fixed crash with certain command alias patterns using wildcards.
   * Devices: Fixed IDEPlus 2.0 to have internal SDX at higher priority than external cart.
   * Devices: Fixed MIDI SysEx message parsing issues with MidiMate emulation.
   * Disk: Tightened validation when mounting MyDOS filesystems.
   * Disk: Fixed crash when expanding archives in filesystems with empty directories.
   * Display: Improved display recovery when secondary monitors are turned on or off.
   * Display: Added workaround for breaking change in Windows 10 build 1709 that caused D3D9 exclusive fullscreen mode to not render.
   * Display: Fixed some reporting and stability issues in custom D3D9 shader effect system.
   * HLE: Enforce A=0 on exit from SIOV acceleration.
   * IDE: Serial number field in identify command data is now padded with spaces.
   * PCLink: Directory search operations now return creation time instead of last write time.
   * POKEY: Serial output data is now truncated when the output clock is stopped.
   * POKEY: Fixed keys not being immediately recognized in raw mode when exiting init mode.
   * UI: Fixed /type not working on cold boot.
   * UI: Fix garbage when pasting text copied from WordPad.
   * VBXE: Attribute map is now constrained to 43 cells rather than a lower limit of 8 pixels horizontally.
   * VBXE: Added emulation of color 0 bug in GR.11 in FX1.24 core.
   * VBXE: Blit pattern width field is now 6 bits instead of 7.
   * VBXE: Overlay priority is reset to $FF at top of XDL.

## Version 2.90: [July 15, 2017]
   ### changes
   * Display: Direct3D 9 bicubic filtered display support has been removed for pixel shader 1.x video cards.
   * Display: Default color presets have been updated.
   * Firmware: The HLE kernel has been removed, as it was out of date and did not have enough advantages. The 800 LLE kernel is now loaded for any profiles that used the HLE kernel.
   * HDevice: The escape character for reserved device name conflicts is now !. $ is still accepted on read.

   ### features added
   * Cartridge: Added /nocartchecksum command line switch for loading .CAR files without an initialized checksum field for development.
   * Cassette: Motor restarts and individual sample timings are now randomized when the randomize option is enabled.
   * Cassette: Automatic rewind on cold reset can now be disabled.
   * CPU: 65C816 CPU speed can now be changed on the fly without a restart.
   * Debugger: History window now shows preview of next instruction.
   * Debugger: Added directive and debugger script support for cartridge images.
   * Debugger: Added 'ir' command to read from input ports that have side effects on read.
   * Debugger: Added .tracesio command.
   * Debugger: Added .crc command.
   * Debugger: db (dump bytes) now takes a -w (width) argument.
   * Debugger: st (static trace) now has a -m option to add a symbol for the starting address.
   * Debugger: Added .logopen and .logclose commands.
   * Devices: Improved usability of Devices dialog.
   * Devices: BlackBox and MIO now support multiple devices on the SCSI bus.
   * Disk: Disk drives dialog now supports drag-and-drop.
   * Disk: Disk drives dialog is now resizable (horizontally).
   * Disk: Initial Happy 810 support (memory read/write commands only).
   * Disk: Added support for full disk drive emulation of 810, Happy 810, 810 Archiver, 1050, 1050 Duplicator, US Doubler, Speedy 1050, Happy 1050, Super Archiver, TOMS 1050, Tiger 1050, 1050 Turbo, Indus GT, XF551, ATR8000, and PERCOM RFD-40S1 drives.
   * Disk: Virtual DOS 2 disk image handler now supports preallocation of blocks to work with track buffering.
   * Disk Explorer: Compressed disk images can now be loaded from the Disk Explorer.
   * Disk Explorer: Read/write access is now allowed to SDFS filesystems that have less serious VTOC/bitmap errors (inaccessible sectors only).
   * Display: Added borderless windowed / windowed fullscreen mode (enabled in Options).
   * Display: Added 16-bit surface option for low-end graphics devices.
   * Display: Sharp bilinear filter mode avoids sharpening horizontally if high artifacting is enabled.
   * Display: Reduced luma bleed-through in NTSC artifacting algorithm and added support for modifying the color conversion matrix.
   * Firmware: Added option to export the internal ROM set.
   * GTIA: Added CTIA emulation mode.
   * HLE: Fast boot now accelerates OS startup when Ultimate1MB is active.
   * IDE: KMK/JZ IDE now has a setting to change the PBI device ID.
   * Input: "Hold keys" now allows console button and key combinations to be held down on the next Reset that might be difficult or impossible to hold down normally.
   * Input: Added a keyboard option to allow host keys to be shared between by the keyboard and input maps.
   * POKEY: Improved SIO transfer logging.
   * Profiler: Tabular data can now be copied to the clipboard via right-click option.
   * Simulator: Added power-on delay setting to simulate turning on the computer later than peripherals.
   * Simulator: Added some optional confirmations before automatic resets.
   * UI: Copy/Save Frame now works for XEP-80 display.
   * UI: Added variant of Copy/Save Frame which uses the true pixel aspect ratio with filtering.
   * VBXE: Implemented config latch.

   ### bugs fixed
   * Additions: Colormap now restores palette properly on exit.
   * Additions: Fixed an uninitialized variable in SX212.COM that could cause speed-switching issues on init.
   * AltirraOS: Modified values of PALNTS for better compatibility with XL/XE OS.
   * AltirraOS: Fix short block flag not being handled by direct SIO calls for the cassette device.
   * AltirraOS: Suppress type 3 poll to disk boot only (fixes Pole Position and Missile Command cartridge audio).
   * ANTIC: Fixed bogus abnormal playfield DMA condition on playfield clock speed switch with blank line in between.
   * ATBasic: Fixed READ line not getting reset after NEW.
   * ATBasic: PMBASE is no longer altered if PMGRAPHICS has not been executed.
   * Cassette: Improved stability of modified bit decoding algorithm.
   * Cassette: Fixed sporadic crash when re-recording in the middle of an existing tape.
   * Cassette: Fixed "randomize" setting not saving properly.
   * Cassette: Acceleration patch now updates the POKEY SERIN register after reading a block.
   * CPU: Fixed broken STZ zp,X instruction in 65C02 mode only.
   * Debugger: Fixed breakpoints not being disabled on detach if simulator was running.
   * Debugger: Blocked rich text paste into command line.
   * Debugger: Disk write from .diskwritesec was not always auto-flushed to disk.
   * Debugger: Static trace (st) and dump disassembly (.dumpdsm) commands now work with coprocessor targets.
   * Debugger: Watches now work with coprocessor targets.
   * Debugger: Heat-map based uninitialized effective address trap is now filtered properly.
   * Debugger: Fixed crash when examining I/O bus memory in the debugger in 800 mode.
   * Devices: Fixed SX212 auto-speed switching issues in the emulated R: handler.
   * Devices: SX212 now powers up in high speed.
   * Disk: Improved timing of SIO burst transfers for more reliable operation at very high speeds.
   * Disk: Added workaround to SDFS formatter and virtual disk handler for unusual disk format check in IDE+2 executable loader.
   * Disk: Adjusted high speed C/E-to-data delay for 1050 Turbo emulation mode.
   * Disk: Adjusted serial timing for Indus GT.
   * Disk: Fixed double density and quad density .XFD disk images.
   * Disk: Fixed disk position not being deterministic on a cold reset.
   * Disk Explorer: Fixed failures when trying to drag a zero byte file from Explorer into a disk image.
   * Display: Fixed saving of non-fullscreen window positioning after exiting in fullscreen mode.
   * Display: Enhanced CIO display now sets cursor position properly on Delete Line.
   * Display: XEP80 display no longer renders with point sampling when filter mode is set to bicubic.
   * Display: Added workaround for display issues on Intel graphics in Direct3D 11 mode.
   * Display: Fixed display not updating scaling properly after toggling VBXE.
   * HDevice: Fixed crash when doing wildcard rename with filename that matches a reserved device name.
   * HLE: Removed DSKINV acceleration hook to work around nonstandard DSCTLN handling in QMEG OS.
   * HLE: Fixed incorrect handling of an accelerated SIO command interrupting a non-accelerated SIO command.
   * HLE: Fixed AFP allowing exponent zero with FP acceleration enabled.
   * PCLink: Fixed characters after ? in a wildcard pattern being ignored.
   * POKEY: High-pass flip flop update was off by one cycle.
   * POKEY: Three-cycle offset between low and high linked timers is now properly maintained in passive timer mode.
   * Serial: Added delay between ATA/ATD and CONNECT to deal with programs that expect to be able to close and reopen R: in between.
   * Simulator: Corrected machine clock rate for SECAM machines.
   * UI: Main window now autoselects a new active pane when the current active pane is undocked.
   * UI: Audio monitor now shows correct frequencies for PAL.
   * VBXE: The xcolor bit now also gates bit 0 of colors in attribute map cells.
   * XEP80: Tab stops are now set properly on soft reset.

## Version 2.81 [October 16, 2016]:
   [bug fixes]
   * Cartridge: Fixed saving SIDE 1 cartridge images.
   * Cartridge: Fixed crash with XEGS cartridge types.
   * HLE: Fixed crash when viewing PBI region in debugger with PBI acceleration disk ROM enabled.

## Version 2.80 [September 3, 2016]:
   ### changes
   * Devices: MyIDE and KMK/JZ IDE (IDEPlus) have been moved to the Devices tree.
   * Input: The preset input maps for 5200 mode and the Xbox 360 Controller been changed.
   * Input: Input mapping speeds have been adjusted; mappings in relative mode may need to be fixed up.
   * UI: Error message help has been moved from Shift+hover to Alt+click.

   ### features added
   * Audio: Added emulation of serial I/O noise with audio channels silent.
   * Cartridge: Added support for alternate image layout of 5200 Bounty Bob cartridge.
   * Cassette: Added support for recording cassette tapes.
   * Cassette: Added option to add slight randomized jitter to tape start position to work around load timing issues with deterministic timing.
   * Covox: Variable base address and mono/4ch. settings added.
   * Covox: Volume is now adjustable (Audio Options).
   * Debugger: All special variables in debugger expressions can now be specified with a @ prefix, i.e. @a. The non-prefixed form is now deprecated due to symbol/number conflicts.
   * Debugger: Focus is now automatically switched between the console and the display when starting and stopping execution.
   * Debugger: Heat map can now be used to detect accesses to uninitialized memory (hmt command).
   * Debugger: Expression parser can now optimize +/-0, *0, and *1 patterns.
   * Debugger: .tapedata command now accepts a length option.
   * Debugger: .basic_dumpline now has a function to show addresses and byte values for each token (-k).
   * Debugger: Added db/dbi -c option to dump mode 1/2 strings.
   * Debugger: Added emulation network packet tracing (.netpcap, .netpcapclose).
   * Debugger: Added .basic_rebuildvvt command to fix corrupted BASIC variable type/index entries.
   * Debugger: PC breakpoints and step execution are now supported for Veronica.
   * Debugger: Added eb (enter byte) and ew (enter word) commands.
   * Debugger: Fixed disassembly of 65C02 RMBn/SMBn/BBRn/BBSn instructions.
   * Debugger: Added .kmkjzide command.
   * Devices: Added Corvus Disk Interface emulation.
   * Devices: Added Pocket Modem emulation.
   * Devices: Added simple joystick dongle emulation.
   * Disk: Added VRWSafe write mode (virtual read/write with format prohibited), and added option to set default write mode.
   * Disk: Added emulation of Indus GT Synchromesh and SuperSynchromesh firmware modes.
   * Disk: Added emulation of full format times when accurate timing is enabled.
   * Disk: Added support for formatting and exploring DOS 1.x disks and for DOS 1.x files on DOS 2.0S disks.
   * Disk: Partial emulation of built-in Happy drive commands.
   * Disk: Added command to expand all .ARC files on a SpartaDOS disk.
   * Disk Explorer: File viewer mode and window position is now persisted.
   * Disk Explorer: SDFS filesystem handler now validates the full directory tree and allocation bitmap.
   * Display: Added /w command line switch to force windowed mode.
   * Display: Added DXGI-based windowed vsync support to D3D9 display driver for reduced CPU usage with DWM composition.
   * Firmware: Added autodetection of 5200 2-port BIOS, Black Box, and MIO ROMs.
   * Firmware: Firmware dialog now supports drag-and-drop.
   * HLE: Program loader now warns on init segment behaviors that may not work with DOS.
   * HLE: Added PBIDisk device for PBI-based acceleration of disk and serial bus requests.
   * IDE: MyIDE, KMK/JZ IDE, and IDE Plus 2.0 now support slave devices.
   * IDE: IDE Plus 2.0 SDX, write protect, and partition switch buttons are now supported.
   * IDE: IDE Plus 2.0 revision and ID switch are now controllable.
   * Input: Improved mouse interrupt timing algorithm.
   * Input: Acceleration can now be adjusted independently of speed for digital-to-analog mappings.
   * Input: Added driving and keyboard controller support.
   * Input: Reset system screensaver timeout when controller input changes.
   * MMU: Added support for 256K Rambo configuration, which aliases 64K with main memory.
   * Network: Added TCP logging channel for monitoring gateway TCP stack status.
   * UI: Added /si and /nosi as synonyms for /singleinstance and /nosingleinstance.
   * UI: Added profile system for switching full or partial configurations.
   * UI: File associations can now be set user-local as well as system-local.
   * UI: Added option to reset all settings.
   * UI: Added option to control whether simulation is paused while menus are open.
   * UI: Detect files dragged from .zip files in Windows Explorer and use a VFS path to the compressed file if possible.
   * VFS: Images mounted within .zip and .gz files are now restored on next load.

   ### bugs fixed
   * AltirraOS: Fixed XEGS game cartridge activation.
   * AltirraOS: Fixed errors getting dropped in cassette handler.
   * AltirraOS: Fixed extra initial block and incorrect partial block problems when writing cassette streams.
   * AltirraOS: Fixed CIO read record when line exactly fits in buffer.
   * AltirraOS: Fixed broken inverse key.
   * AltirraOS: S: clear also clears split-screen area.
   * AltirraOS: Optimized C/E->data frame path in SIO for better robustness when DLIs are active.
   * AltirraOS: Fixed race condition in SETVBV.
   * ANTIC: Fix power-up values for PENH and PENV registers.
   * ATBasic: Added workaround for BASIC programs that have a corrupted function open parens.
   * ATBasic: Force implicit NEW if LOAD or CLOAD fails.
   * ATBasic: Fix crash in INT() with certain ranges of large numbers.
   * ATBasic: Remove added variables when parsing error occurs.
   * Cartridge: Changed initial bank for XEGS/SXEGS carts to 0.
   * Cartridge: Fixed flash indicator not always appearing while programming The!Cart cartridges.
   * CPU: Fix D flag not being cleared on interrupt entry in 65C02/65C816 mode and I not being cleared in 65C816 native mode.
   * CPU: Fixed second write cycle for read/modify/write instructions in 65C816 emulation mode.
   * Debugger: Fixed assembly and disassembly of 65C816 COP instruction from COP #n to COP n.
   * Debugger: Fixed issue with Step Over breaking PC breakpoints.
   * Debugger: PC breakpoints are now bank sensitive.
   * Debugger: Fixed assembler giving branch range errors when assembling above bank 0.
   * Debugger: Fixed disassembly of 65C02 RMBn, SMBn, BBRn, and BBSn instructions.
   * Display: Fix for intermittent display redraw issues when stopped in debugger with display composition enabled.
   * Display: Fixed centering of enhanced text mode after switching modes.
   * Display: Enhanced text mode (hardware) now uses correct colors for Gr.1/2.
   * Display: Fixed excessive CPU usage with enhanced text mode with debugger open.
   * Display: Removed broken fixed function bicubic stretching code paths. They were broken and pre-shader cards don't have the fill rate to do 5+ passes at 60fps anyway.
   * Disk: Fixed density detection issue with Set PERCOM Block command in XF551 mode (again).
   * Disk: Fixed crash when mounting .ARC file with no decodable files in it.
   * Disk: Retuned command ACK timings for 1050-based drives.
   * Disk: 810 and 1050 emulation modes now implement proper respective behavior with long sectors.
   * Disk: Allow loading of truncated .ATRs with a partial final sector.
   * Disk: Fixed handling of sectors with bad address field CRCs.
   * Disk: Fixed record type bits in FDC status in 810 mode.
   * Disk Explorer: Fixed rename operations on SDFS disks not marking volume changed.
   * Flash: Toggle bits are now implemented during the multiple sector erase timeout period.
   * HLE: Improved register return state when accelerating ZFR0/ZF1/ZFL calls.
   * IDE: Fixed excessive flushing of VHD block bitmap after allocating space.
   * IDE: Force power and reset states on MyIDE-II when CF card is removed.
   * IDE: Improved compatibility of CHS mapping behavior.
   * IDE: IDEPlus 2.0 now supports external cartridge control.
   * Input: Fixed mouse being able to escape from capture at high speeds.
   * Input: Digital-to-analog mappings no longer change speeds between NTSC and PAL.
   * Input: Fixed preset maps sometimes not resetting if they were modified in the same session.
   * Input: Fixed 5200 CONSOL bits being inconsistent internally after reset (fixes 5200 Pole Position controller input).
   * Input: Fixed 5200 trackball controller behavior when bound to mouse move inputs.
   * MMU: Fixed case where MMU was not reset when switching from 400/800 mode to XL/XE mode, causing a power-up crash.
   * MMU: Axlon memory no longer aliases with PORTB extended memory.
   * Network: Fixed TCP stack checking wrong sequence number against incoming ACK in some cases.
   * Network: Improve TCP PSH and ACK handling.
   * Network: Fix inability to write some CS8900A control registers through PacketPage ports.
   * PCLink: Fixed incorrect return codes from rename command.
   * PCLink: Directories are now readable as byte streams.
   * POKEY: Improved emulation of fast pot scan mode.
   * POKEY: Improved RANDOM emulation when switching in and out of init mode.
   * POKEY: Fixed serial output ready IRQ occasionally not triggering when expected.
   * SIDE2: Implemented CF change detection.
   * SIDE2: The right cart window of the top cartridge half can now be enabled without the left half.
   * Simulation: Modified DRAM A randomization pattern in 5200 mode to match 5200's address line mapping.
   * UI: Disable Shift+hover help when Shift key is bound to an input map.
   * UI: The kernel firmware menu is now filtered to kernels compatible with the current hardware mode.

## Version 2.71 [April 16, 2016]:
   ### bugs fixed
   * Added workaround for crash on XP systems without SP3 or KB921337 hotfix installed.

## Version 2.70 [December 19, 2015]:
   ### changes
   * Debugger: Switched BRK back to being a single byte opcode in the disassembler.
   * Debugger: Readjusted names of illegal opcodes.
   * Devices: SoundBoard has been moved to the Devices tree.
   * Input: Because some broken axis mappings have been corrected, hand-edited input maps may need to be fixed up.

   ### features added
   * ATBasic: Added LOMEM, ERR(), PMGRAPHICS, PMADR(), PMCLR, PMCOLOR, PMMOVE, MISSILE, and BUMP().
   * Audio: Sound output is auto-switched to 48KHz to reduce CPU usage if Windows is mixing at that rate.
   * Debugger: Added ~ command (target control).
   * Debugger: bx (break on expression) can now create tracepoints (-n).
   * Debugger: Added Go To Source from History window.
   * Debugger: .basic_dumpline command now has -t option for decoding TurboBasic XL tokens.
   * Debugger: Added Go until vertical blank interrupt (gv) command.
   * Debugger: Execution history now shows high level emulation events.
   * Debugger: Added sw (search words), sa (search ATASCII), and si (search INTERNAL).
   * Debugger: Added %S format to .printf for high byte terminated strings.
   * Debugger: Added @frame, @clk, and @cclk expression psuedovariables.
   * Devices: Add minimal SDrive raw disk access emulation.
   * Devices: Veronica cartridge emulation.
   * Devices: R-Verter emulation.
   * Disk: Added 1050 drive sounds and added emulation of 1050 reseeking on errors.
   * Disk: Added "shift to another drive" option to disk dialog.
   * Display: Optimized NTSC high artifacting mode.
   * Display: Improved performance of D3D11 vsync code.
   * Display: Enhanced text mode now works in full screen mode and supports selection.
   * HLE: Added player for some SAP type B/C/R files.
   * IDE: Raised VHD creation size limit in UI from 4GB to 2TB.
   * IDE: Added support for MyIDE II with updated CPLD firmware.
   * Input: Dead zones and curves are now adjustable for game controllers.
   * Input: Keyboard mapping is now customizable.
   * Profiler: Added Basic block profiling mode.
   * Profiler: Added performance counter support.
   * Profiler: Instruction-level data is now recorded even in function mode.
   * Profiler: Frame triggers can now be set to record multiple frames.
   * Recorder: Added support for recording SAP type D files.
   * SIO: Merged standard and polled burst I/O into a single mode.
   * SIO: Burst I/O is now supported for writes.
   * Simulator: Emulation is no longer paused while menus are open.
   * UI: Audio monitor now supports dual POKEYs.
   * UI: Paste now attempts to translate Unicode characters like curly quotes and dashes to representable characters.
   * UI: Raised drag-and-drop limit to 128MB to accommodate The!Cart images.
   * UI: Portable mode now stores relative paths for paths above the program directory on the same drive root.

   ### bugs fixed
   * 65C02: Added missing BIT abs,X instruction (65C02 mode only).
   * 65C816: Stack instructions new to the 65C816 now properly index outside of page one in emulation mode.
   * 65C816: Fixed PLY instruction not working outside of page one in X8 mode.
   * 65C816: Fixed WAI releasing execution too early on IRQs in high speed mode.
   * ANTIC: Fixed blanking of rows 8-9 in IR mode 2.
   * ANTIC: Fix incorrect internal state after warm reset during WSYNC wait.
   * AltirraOS: Activate self-test ROM after memory test to mimic the XL/XE OS's behavior when doing ROM checksums.
   * AltirraOS: Improved compatibility of S: plot/line commands in GR.0.
   * AltirraOS: E:/S: open now enables keyboard IRQs.
   * AltirraOS: Fixed BOOT? flag being set too early on cassette/disk boots.
   * AltirraOS: CIO PUT RECORD with length=0 now outputs A without EOL.
   * AltirraOS: XL/XE version now supports KEYREP.
   * AltirraOS: Caps Lock now toggles in XL/XE mode.
   * ATBasic: Fixed bug with terminating EOLs being left in code if a warm reset happened during a filename-based I/O statement.
   * ATBasic: Fixed CLOAD/CSAVE not setting IRG mode consistently.
   * Cartridge: Fixed crash with MegaCart 1M (2) mapper.
   * Cartridge: Fixed crash problem with RAM access in newly created The!Cart cartridges.
   * Cartridge: Newly created AtariMax 8Mbit (bank 0) carts had the wrong power-up bank.
   * Cartridge: Fixed random crash with Atrax SDX cartridge types.
   * Debugger: Fixed Step Over not working in 65C816 mode.
   * Debugger: Fixed B/P/S registers being shown incorrectly in history view in 65C816 mode.
   * Debugger: Improved call stack decoding.
   * Debugger: Fixed assembly/disassembly of MVP/MVN instructions.
   * Debugger: Fixed assembly of [dp],Y addressing modes.
   * Debugger: Fixed crash when canceling edit of watch expression.
   * Debugger: Fixed disassembly of BIT abs,X instruction in 65C02 mode.
   * Debugger: Fixed .readmem command.
   * Debugger: Up/down keys now work better in History when navigating search results.
   * Debugger: Fixed time wrapping bug in .pokey deferred timer reporting.
   * Debugger: Fixed length (Lxxx) arguments not accepting 10000 for $10000 bytes.
   * Debugger: Fixed crash with malformed read/write access range checks in breakpoint conditional expressions.
   * Debugger: Fixed assembler not accepting PHX, PHY, RTL, TCS, and TXA instructions.
   * Debugger: Fixed assembler not assembling some instructions with [dp] and relative long addressing modes.
   * Debugger: Assembler no longer uses the debugger's numeric base setting and always assumes decimal unless $ is prefixed before a number.
   * Disk: Fixed boot sector not being initialized when precreating formatted DOS 3 disks.
   * Disk: Data checksum errors during SIO bus transfers now result in a NAK instead of Error.
   * Disk: Improved accurate sector timing values for enhanced density disks.
   * Disk Explorer: Fixed crash when activating context menu with no disk image mounted.
   * Disk Explorer: Empty filenames are no longer allowed.
   * HDevice: Fixed regression in NOTE/POINT commands.
   * Input: Fixed right mouse button on mice not working for ports 2/4.
   * Input: Fixed inconsistent mappings between analog/digital interpretations of axes and between DirectInput and XInput.
   * Network: Fixed protocol handling bugs in emulated ARP and DHCP layers.
   * Network: Add support for tunneling over the VXLAN protocol.
   * Simulation: Added timeout to logic for holding Option on boot.
   * Simulation: Added emulation of floating I/O memory bus on 800 hardware.
   * PCLink: Fixed FLEN (code 4) command.
   * PIA: Interrupt flags can now be set without having interrupts enabled.
   * PIA: Reenabling IRQA1 or IRQB1 with the flags already set now reasserts IRQ.
   * PIA: Added emulation of floating inputs on PIA port B on XL/XE systems.
   * POKEY: Serial input no longer works while initialization mode is active.
   * POKEY: Full keyboard scan emulation is now supported, including phantom key and debounce-off effects.
   * Recorder: Video recording failed if scanlines were enabled without VBXE or antialiasing on.
   * U1MB: Removed bogus PORTB dependency for register access.
   * U1MB: Flash ROM is now reset on cold reset.
   * U1MB: Flash writes to OS, BASIC, and Game ROM windows are now supported.
   * U1MB: Real-time clock now properly allows both clock polarities.
   * U1MB: Fixed value of reserved bits read from RTCIN register.
   * U1MB: Internal BASIC now works with 1200XL + U1MB, since the U1MB emulates the XL/XE MMU.
   * UI: Fixed /portablealt writing into the current directory instead of starting directory when given a relative path.
   * UI: Changed cursor mode to fix drag-and-drop incompatibility with 7-Zip File Manager.
   * UI: Audio monitor setting now saves.
   * UI: Selecting a 1200XL kernel no longer switches the hardware mode to 800XL.
   * UI: Fixed firmware options not refreshing consistently in dialog.
   * XEP80: Fixed UART register emulation.
   * XEP80: Attributes, split-screen, and vertical scrolling are now supported in pixel graphics mode.

## Version 2.60 [March 21, 2015]:
   ### changes
   * Devices: PCLink, DragonCart, XEP-80, SlightSID, Covox, R-Time 8, 850, 1030, P:, and H: have been moved to the Devices tree.

   ### features added
   * ATBasic: Improved execution speed.
   * ATBasic: Added partial support for CONT statement.
   * ATBasic: Added support for DPOKE, BPUT, BGET, ERASE, PROTECT, UNPROTECT, DIR, RENAME, MOVE, HEX$(), DPEEK(), !, %, &, and $.
   * AltirraOS: Optimized FPI routine.
   * AltirraOS: Improved rounding of FDIV results.
   * BlackBox: Initial emulation support.
   * Debugger: Changed .writemem to use length syntax.
   * Debugger: .basic_dumpline updated with shortcuts for STMCUR/STMTAB and continuous output.
   * Debugger: Added .basic_rebuiltvnt command.
   * Debugger: Added support for one-shot, grouped, and clear-on-reset breakpoints.
   * Debugger: Added support for return tracepoints (bt --).
   * Debugger: Added .sprintf command and @ts tokens.
   * Debugger: Added @t0-t9 (temporary) and @ra (return address) variables.
   * Debugger: Added option to randomize application memory after EXE load (direct EXE load only).
   * Debugger: Added .diskreadsec and .diskwritesec commands.
   * Debugger: Added dbi (dump bytes with INTERNAL text) command.
   * Devices: Added support for SIO-based real-time clock devices.
   * Devices: Added SIO type 3/4 polling test devices.
   * Devices: Added SX212 modem emulation.
   * Devices: Added MidiMate emulation.
   * Disk: Added support for creating and exploring DOS 3 disk images.
   * Disk: Added generic emulation mode with 57,600 baud high speed support.
   * Disk: ATX, PRO, DCM, and XFD disk images can now be written.
   * Disk: Detach Disk commands now turn off disk drives.
   * Disk: Added rotate disk commands.
   * Disk Explorer: Added read/write support for MyDOS and DOS 2.5 disks.
   * Disk Explorer: Added create directory (new folder) option for MyDOS and SDFS disks.
   * Display: Sharp bilinear now works on pixel shader 1.x devices.
   * Display: Vertical overscan can be overridden separately from horizontal.
   * HDevice: Added option to use lowercase filenames.
   * HDevice: H: can now be hot-started or stopped without a reboot.
   * HDevice: H: can now also be installed as D:.
   * IDE: CF hot swapping is now supported.
   * Input: Added support for quick-cycling between a set of input maps.
   * MMU: Axlon banking register aliasing is now optional.
   * Printer: Added SIO-level 820 emulation.
   * Serial: Modem now supports RTS/CTS flow control (&K3).
   * Serial: Added software T: handler and support for 1030 handler bootstrap.
   * Simulation: Added support for mixed NTSC/PAL ANTIC+GTIA combinations (PAL-60 and NTSC-50).
   * Simulation: Added support for more realistic power-on memory patterns.
   * Software: Added Additions disk containing supplementary utilities.
   * UI: Added new firmware management dialog and support for multiple custom OS and BASIC ROMs.
   * UI: Added /[no]cheats switch.
   * UI: Improved high DPI support.
   * UI: Added /portablealt:<file> switch to use alternate INI files.
   * UI: Multiple disk images can now be loaded with repeated /disk switches.
   * UI: Full screen mode state is now saved on exit.
   * UI: Improved full-screen UI.
   
   ### bugs fixed
   * ATBasic: Fixed precedence of unary minus vs. exponentiation.
   * ATBasic: Fixed x^y for x<=0.
   * ATBasic: Error messages now indicate STOPPED or line numbers as appropriate.
   * ATBasic: Fixed not being able to Break out of a single-line loop.
   * ATBasic: TRAP line is now reset more appropriately.
   * ATBasic: Fixed range reduction in SIN() and COS().
   * ATBasic: GRAPHICS no longer reopens E:.
   * ATBasic: Fixed CONT sometimes failing in deferred mode.
   * ATBasic: Fixed parsing errors on the right side of a string relational expression.
   * ATBasic: Banner changed so startup programs that hardcode line 2 work.
   * ATBasic: Use of IOCB #0 is no longer allowed.
   * ATBasic: Out of memory errors are now detected and thrown.
   * ATBasic: PUT now takes an aexpr instead of avar.
   * ATBasic: PTABW is now supported.
   * AltirraOS: SIO no longer saves STACKP before invoking PBI routines (fixes BlackBox crash).
   * AltirraOS: SIO now leaves ICBALZ/ICBAHZ equal to ICBAL/ICBAH for better custom DOS compatibility.
   * AltirraOS: Fixed CRITIC being left on after servicing a PBI-based SIO request.
   * AltirraOS: Fixed GET BYTE returning bogus characters for PBI-based CIO devices.
   * AltirraOS: E: now supports forced read mode.
   * AltirraOS: SETVBV now preserves I flag.
   * AltirraOS: AUX1/2 are now updated properly on R: XIO commands.
   * AltirraOS: Fixed CIO type 4 polling support.
   * AltirraOS: Fix timeout for SIO operations.
   * AltirraOS: Fix ESC-Clear handling in E:.
   * AltirraOS: Cassette boot now reads EOF record.
   * AltirraOS: Fixed diagnostic cartridge handling.
   * ANTIC: Improved emulation of phantom P/M DMA effects.
   * Cassette: Fixed inconsistent motor control state when using C: patch with short IRG mode.
   * CPU: Fixed NMI not triggering early enough when occurring during IRQ entry sequence.
   * CPU: Fixed spurious IRQs when interrupts are masked for more than 2^32 cycles.
   * CPU: Fixed video snow problem when running CPU faster than 1.79MHz.
   * Debugger: -$80000000/-1 no longer causes a crash.
   * Debugger: .printf %u formatter now works correctly for numbers >=10^10.
   * Debugger: Fixed EXP/EXP10 not being logged to FPACCEL channel.
   * Debugger: Fixed static trace (st) not stopping properly on BRK/RTS/RTI instructions.
   * Debugger: Debug display now reflects CHACTL bits.
   * Disk: Rewrote ATX image parser to be more robust.
   * Disk: Added workaround for XFD images improperly named .ATR.
   * Disk Explorer: Fixed DOS 2 filename encoding for filenames without extensions.
   * Disk Explorer: Fixed directory parsing for double-density DOS 2 filesystems.
   * Display: Fixed crash on display hardware change when using DirectDraw.
   * Display: Added workaround for crash when initializing Direct3D 9 with VirtualBox display driver.
   * Display: Improved windowed mode vsync strategy when DWM is active.
   * Display: Improved quality of NTSC high artifacting mode.
   * GTIA: Fixed DMA reads from hardware registers.
   * HDevice: Fixed NOTE command sometimes giving incorrect positions.
   * HLE: Hook page for H:, P:, R:, T:, and E: hooks is now better at dodging conflicting hardware windows.
   * HLE: Fixed EXP10(-1) returning an error with FP acceleration enabled.
   * HLE: Improved register return values for several FP acceleration calls.
   * HLE: SIO patch acceleration is skipped if the I flag is set.
   * Input: Fixed right mouse button not working on mouse controllers.
   * Input: Keyboard and mouse inputs are now auto-released when display window loses focus.
   * Input: Moved joystick poll before VBI to reduce input delay.
   * Network: Fixed emulated Ethernet packet timing.
   * POKEY: Undetectable $C0-C7 and $D0-D7 scan codes are now blocked.
   * Serial: 6502-based R: handler now waits for output buffers to drain on a close command.
   * Serial: Fixed emulated R: device only draining one byte from transfer buffer per call in unthrottled mode.
   * Serial: Many fixes to 1030 T: device command handling.
   * UI: Fixed mouse pointer blinking when a video player injects mouse moves to prevent the display from sleeping.
   * Ultimate1MB: Fixed SDX control register not gating non-SIDE external cartridges properly.
   * XEP80: Move Cursor to Start of Logical Line ($DB) no longer moves cursor to left margin.
   * XEP80: Set horizontal/vertical position commands now update the cursor address.

## Version 2.50 [July 26, 2014]:
   ### features added
   * BASIC: Altirra BASIC is now the default when no BASIC ROM is included.
   * Cartridge: Added support for another 512K/4MB/8MB flash cartridge.
   * Cartridge: Added support for The!Cart and MegaMax 2M cartridge types.
   * Cassette: Tape control UI now has a waveform graph.
   * Cassette: Added support for FSK blocks in .cas files.
   * CPU: Extended 65C816 speed options to include 17MHz (10x) and 21MHz (12x).
   * Debugger: Register (r) command can now set the emulation (E) flag.
   * Debugger: Improved display of 65C816 register state.
   * Debugger: Disassembly window now steps properly above bank $00.
   * Debugger: Added search capability to History pane.
   * Debugger: Added some 65C02 and 65C816 instructions to assembler.
   * Debugger: Added .loadobj command.
   * Debugger: Keyboard shortcuts are now configurable.
   * Debugger: .basic_dumpline now decodes BASIC XL/XE tokens.
   * Debugger: Added x (examine symbols) command.
   * Debugger: Added module!symbol syntax for resolving symbols specific to a module.
   * Debugger: .sdx_loadsyms command now works on more versions of SpartaDOS X.
   * Debugger: .printf now supports string output and variable width/precision.
   * Debugger: Failed assertions now report file/line info if available.
   * Debugger: Added .basic_dumpstack command to dump BASIC runtime stack.
   * Debugger: Added option to hide namespaces for nested symbols in history window.
   * Debugger: .dumpsnap command now applies compression.
   * Debugger: Added partial CC65 dbgfile support.
   * Disk: .ARC files can now be mounted as SpartaDOS X disk images.
   * Disk: Added option to format new disks with the SpartaDOS File System (SDFS).
   * Disk: Added support for 8KB sectors.
   * Disk Explorer: DOS2 filesystem validator no longer checks directory entries after end of directory.
   * Display: Fixed update problems when stepping with frame blending active.
   * Input: Middle mouse button (MMB) can now be used to release mouse capture if it is not already bound.
   * Input: Added XInput support.
   * Network: DragonCart emulation support.
   * POKEY: Audio output path now emulates analog decay effects.
   * Printer: Added Clear option to printer output pane.
   * Profiler: Added columns for unhalted cycles.
   * Serial: Service field in ATDI command is now optional and defaults to telnet (port 23).
   * Serial: Added Telnet terminal type negotiation support.
   * Serial: Added Telnet binary transfer support.
   * Serial: Added full 850 Interface Module SIO protocol and handler support.
   * UI: Added some touch device support.
   * UI: Added Attach Disk and Detach Disk menu options.
   * UI: Added direct keyboard layout mode.
   * Ultimate1MB: Added SST39SF040, Am29F040B, and BM29F040 as alternate flash chip options.

   ### bugs fixed
   * 65C816: RTI can no longer clear the X flag in emulation mode.
   * Cartridge: Fixed swapped 5200 two-chip and one-chip modes.
   * Cartridge: Cold reset now resets flash emulation.
   * Cartridge: Fixed some cases where flash writes would stop working with emulated MaxFlash cartridge.
   * Cartridge: Fixed incorrect flash device ID for MaxFlash 1MB+MyIDE cartridge.
   * CPU: Fixed crashes when disabling heat map tracking.
   * CPU: Reverted unintentional change to IRQ timing (affected PM 2.0).
   * CPU: Fix hang when saving state with heat map activated.
   * Debugger: lm (list modules) and .unloadsym now work by IDs instead of indices to avoid index renumbering problems.
   * Debugger: Improved M/X tracking in disassembly window.
   * Debugger: Fixed disassembly of absolute long addressing mode.
   * Debugger: Memory window now updates properly above bank 0.
   * Debugger: Fixed memory access value reporting above bank 0 in step disassembly.
   * Debugger: Fixed infinite loop bug in static trace (st) command.
   * Debugger: CC65 label parser now strips the first leading period from label names.
   * Debugger: Fixed truncated call stack when encountering reused function epilog code paths.
   * Disk: Fixed crash when host file is modified on virtual SDFS mounted path and accessed before emulator can notice the change.
   * Disk: Fixed a bug that caused some root files on virtual SDFS disks to not hot-update reliably.
   * Disk: Writes to virtual disks are now blocked.
   * Disk: Fixed density detection issue with Set PERCOM Block command in XF551 mode.
   * Disk Explorer: SDFS directory entries are now reused properly.
   * Disk Explorer: SDFS sparse files are now supported.
   * Disk Explorer: Dragged files are no longer placed in the root when viewing a subdirectory.
   * GTIA: Fixed 2cclk transitions from modes 9/10/11 to mode 8.
   * HDevice: Files can now be created in append mode.
   * HDevice: Improved error handling in delete, rename, lock, and unlock operations.
   * HLE: Acceleration hooks no longer fire in 65C816 native mode or with a non-standard emulation mode environment.
   * HLE: Program loader now detects and rejects SpartaDOS X executables.
   * HLE: Fixed state of carry flag when exiting DSKINV after failure (fixes Micropainter with SIO patch enabled).
   * HLE: Fixed EXP(-1) returning an error with FP acceleration enabled.
   * HLE: Added alternate hooking mechanism so that CIO device hooks can activate on OSes that don't use the standard CIOINV vector.
   * Input: Relative binding to the axis 0/1 inputs of a tablet controller now works.
   * Input: Fixed mouse capture malfunctioning when activated by keyboard shortcut when the mouse is outside of the window.
   * LLE: Writes to S: in mode 0 now extend logical lines properly.
   * LLE: CIO now checks permissions on get/put operations.
   * LLE: Improved accuracy of power series for ATN(x).
   * PCLink: Fixed race condition in completion of fwrite() command.
   * POKEY: Fixed POTn registers changing without POTGO being strobed.
   * Printer: High bit is now stripped on printed characters so that inverse characters can be read instead of becoming ?.
   * Serial: Fixed escaping of $FF bytes over Telnet protocol.
   * Serial: Cold reset now forces modem back to command state.
   * Serial: Modem command handling is now case-insensitive.
   * Serial: Fixed control line status from status commands issued between OPEN and XIO 40 (was causing ForemXEP drops).
   * SIDE2: Fixed SDX banking register address.
   * Simulation: Fixed some issues with GTIA and POKEY state desyncing across save states.
   * UI: Shift modifier is allowed again for Start/Select/Option.
   * VBXE: Extended color bit now enables LSB of GTIA color registers.
   * VBXE: Fixed blit mode 4 (bitwise AND) not working properly with src=0.
   * VBXE: Fixed blitter collision behavior with dest=0.

## Version 2.40 [November 1, 2013]:
   ### features added
   * 65C816: Optimized mode switches.
   * Audio: Added drive sound volume level option.
   * Cartridge: Added support for .CAR types 53-59 (2K, 4K, right-as-left 8K, right slot 4K, 128-512K SIC!).
   * CPU: Preliminary support for accelerated 65C816 operation.
   * Debugger: Added .dmabuf command.
   * Debugger: Added %e, %f, and %g formats to .printf command.
   * Debugger: Verifier can now detect 64K address space index wrapping and abnormal DMA conditions.
   * Debugger: Added fbx (fill bytes with expression) command.
   * Debugger: r (register) command now allows access to 65C816 registers.
   * Disk: Added "Extract Boot Sectors" command to disk dialog for use with bootable virtual disks.
   * LLE: Added PBI device interrupt support.
   * MMU: High (65C816) memory can now be adjusted from 0KB-4032KB.
   * Profiler: Added 65C816 support.
   * Recorder: Added .WAV file audio recording.
   * Recorder: Added option for encoding duplicate frames as full frames.
   * UI: Added on-screen indicators for console buttons held on startup.
   * UI: Added on-screen indication for some view mode changes.
   * UI: Added support for per-monitor DPI scaling in Windows 8.1.
   * UI: Added custom debug font dialog for half point sizes.
   * UI: File > Exit now confirms if there are modified images.
   * XEP80: Initial support.
   
   ### bugs fixed
   * 5200: Floating data bus is now enabled in 5200 mode.
   * 65C816: Fixed cycle timing for JMP (abs) instruction.
   * 65C816: Fixed cycle timing for TXY instruction.
   * 65C816: Fixed TYX instruction.
   * 65C816: INX was checking M bit instead of X bit.
   * 65C816: Read/modify/write instructions now do read/write/write in emulation mode.
   * 65C816: Fixed (dp), (dp,X), and (dp,Y) behavior with DP!=0.
   * ANTIC: Disabling playfield DMA after playfield start now reads bus data into the line buffer.
   * ANTIC: Abnormal playfield DMA is now emulated.
   * ANTIC: Improved precision of CHACTL changes.
   * Cartridge: Fixed $BFxx reading with 5200 64K cartridge type.
   * CPU: Illegal instruction option now saves correctly.
   * Debugger: Display float (df) command displays all ten significant digits.
   * Debugger: Fixed LLE kernel ROM auto-reload and symbol load option.
   * Debugger: UI panels are now more consistent with debugger commands in numeric base handling.
   * Debugger: Fixed incorrect disassembly on step when running from high banks.
   * GTIA: Fixed bug with VDELAY on missiles.
   * GTIA: Fixed regression with hires player-playfield collisions (since 2.30).
   * LLE: Fixes and optimizations to math pack.
   * LLE: Decimal flag is now cleared before dispatching IRQs.
   * LLE: Fixed BRK handler to handle stack wrapping.
   * LLE: 5200 BIOS now strobes NMIRES for DLIs.
   * LLE: Fixed CIOINV timing so that emulated CIO hooks work.
   * LLE: Corrected K: debounce logic and E: AUX2 open handling (fixes Action! with LLE firmware).
   * HLE: Fixes to math pack acceleration.
   * HLE: Added partial fix for CDTMA1 during accelerated disk reads (fixes Ankh with SIO patch enabled).
   * IDE: Fixed value of Sector Count register after READ SECTOR and WRITE SECTOR commands.
   * MMU: Fixed aliasing of high memory banks.
   * POKEY: SKSTAT bit 1 is now emulated.
   * Printer: Emulated P: device now supports the PUT CHARS command with len=0.
   * Serial: Emulated R: device supports break interrupts.
   * UI: Fixed GDI handle leak in text editor.
   * UI: Added workaround for set file associations dialog not appearing on Windows 8.
   * UI: Fixed ANTIC DMA visualization mode with extended PAL height.
   * UI: Fixed PCLink indicator not updating.
   * U1MB: Fixed PIA read decoding to only respond to $D300-D37F (unfixes Bounty Bob Strikes Back!).

## Version 2.30 [May 14, 2013]:
   ### features added
   * Cartridge: SIC! cartridges can now be switched between Am29F040B and SST39SF040 flash chip emulation in Options.
   * Debugger: History pane supports horizontal scrolling and hiding S/P registers.
   * Debugger: Added .readmem command.
   * Debugger: .basic_dumpline command now handles DATA statements and decodes variable names.
   * Debugger: The ESC key moves from several debugger panes to the Console pane.
   * Debugger: Added set tracepoint (bt) command.
   * Debugger: Added deref signed dword (dsd) operator.
   * Debugger: Added brief (-b) format to .sio command.
   * Debugger: .tapedata command now supports seeking options and reports gaps.
   * Debugger: Added .reload command.
   * Disk: Folders can now be mounted as virtual disk images using the SpartaDOS file system.
   * Display: Normal overscan mode now only shows 224 scanlines in NTSC mode instead of 240.
   * Display: Altered full screen resolution selection to allow possibly unsupported modes for better access to 50Hz refresh modes.
   * IDE: Added SIDE 2 support.
   * MMU: Added support for Axlon banked memory.
   * Simulation: Split XL/XE hardware into separate XL and XE modes to handle hardware differences.
   * UI: Full-screen mode can now be used when paused.
   * UI: Added full-screen versions of the Boot Image file browser and emulation error dialog.
   * UI: Added initial UI bindings for controllers (under the Console controller).
   * UI: Added on-screen keyboard (activated by UI Option from main screen).
   * UI: Improved text rendering quality.
   * UI: Right-drag drop brings up a context menu with mounting options.
   * UI: Panes can now be stacked with tabs.
   * VBXE: Updated core support to 1.24, including a/r flag in bit 7 of the MINOR_REVISION register.

   ### bugs fixed
   * 65C816: The abs,X, abs,Y, (dp),Y, [dp],Y and (d,S),Y addressing modes can now cross bank boundaries.
   * 65C816: An additional instruction is no longer executed after WAI before servicing the interrupt.
   * 65C816: JMP (abs) now always reads from bank 0 instead of the data bank.
   * 65C816: JMP (abs,X), and JSR (abs,X) now read from the program bank rather than the data bank.
   * 65C816: Fixed switch between DL=0 and DL!=0 in emulation mode not taking effect immediately.
   * 65C816: Indexed modes now properly take an additional cycle for writes and for 16-bit index registers.
   * 65C816: The d,S addressing mode can now index out of page one in emulation mode.
   * 65C816: d,S and (d,S),Y now always read from bank 0.
   * 65C816: 16-bit stack operations can access page zero and page two in emulation mode.
   * 65C816: TCS/TXS can no longer change SH in emulation mode.
   * Debugger: .tracecio now reports correct status.
   * Debugger: Fixed an issue with stack overflow in History pane on very deep recursion.
   * Debugger: Verifier now allows jumps to the CIO device initialization vectors.
   * Debugger: Fixed incorrect wrapping of zero page indexed addressing modes in disassembly.
   * Debugger: Threaded static trace (st) command.
   * Debugger: e (enter) now follows normal expression evaluation rules.
   * Debugger: Fixed disassembly of PEA ($F4) opcode.
   * Debugger: PHX and PHY no longer create bogus call trees in the History window in 16-bit indexing mode.
   * Debugger: Fixed disassembly of opcodes with the (d,S),Y addressing mode.
   * Debugger: Corrected off-by-one index handling in .unloadsym command.
   * Debugger: Fixed blowup with memory access breakpoints in GTIA range with VBXE enabled.
   * Disk: Status byte 1 bit 5 is now updated after a Write PERCOM Block command.
   * Display: Fixed a crash when Direct3D 11 is enabled and fails to initialize.
   * Display: Fixed alignment errors in OS Screen Only mode with extended PAL height enabled.
   * Display: Enabled frame skip and reduced latency in full screen vsync mode.
   * GTIA: Player/missile size code %10 can now trigger shifter lockup.
   * GTIA: Improved accuracy of mid-shift size changes.
   * HLE: Fixed editor tab commands.
   * HLE: Fixed bug with init segment dispatching.
   * IDE: READ SECTOR and WRITE SECTOR commands now allow up to 256 sector transfers.
   * IDE: READ MULTIPLE and WRITE MULTIPLE commands now produce errors when multiple mode is disabled.
   * IDE: The block size set by the SET MULTIPLE MODE command is now reflected in IDENTIFY DEVICE output.
   * Input: Fixed MultiJoy #8 input conflict.
   * LLE: Fixed system reset NMI handling.
   * LLE: Implemented PBI handler and CIO support.
   * LLE: Keyboard routines now update ATACHR.
   * LLE: Fixed editor tab set/clear commands.
   * LLE: Added printer (P:) device support.
   * LLE: Added light pen/gun support.
   * MMU: Fixed crashes accessing high 65C816 memory in the x64 version.
   * MMU: VBXE can now properly overlay Covox.
   * POKEY: Fixed timers 3+4 occasionally not being shut off when switching to asynchronous receive mode.
   * POKEY: Fixed timers 1-3 having the wrong offset when underflowing immediately before a write to STIMER.
   * POKEY: Corrected start timing of polynomial counters.
   * POKEY: Corrected audio pattern for 9-bit and 17-bit polynomial counters (was incorrectly inverted).
   * Recorder: Fixed 8-bit uncompressed output.
   * Recorder: Minor fixes to compressor.
   * Serial: SIO emulation level setting is now saved correctly.
   * VBXE: Writing a '1' to the blitter start bit no longer aborts an active blit.

## Version 2.20 [January 6, 2013]:
   ### features added
   * UI: Overlays are now rendered through composition at native resolution instead of simulated resolution.
   * UI: Text can now be copied out when the simulation is stopped.
   * UI: Keyboard shortcuts are now configurable.
   * UI: Drag-and-drop from compressed folders in Explorer is now supported.
   * UI: Shift-hover over a BASIC or DOS error message now displays error help.
   * UI: Mouse cursor changes when light pen/gun is moved on and off the virtual screen.
   * Debugger: Holding SHIFT with the mouse over the display pane now shows the beam coordinates for that point.
   * Debugger: Added .pbi and .ultimate1mb commands.
   * Debugger: Extended many commands to accept expressions for address and length arguments.
   * Debugger: The default numeric base for addresses can now be changed (.base).
   * Debugger: Added extended memory bank variable to expression parser.
   * Debugger: Symbols using reserved expression operator names can now be escaped with #.
   * Debugger: e (enter) command now accepts extended addresses.
   * Debugger: .map command now displays layer names.
   * Debugger: bs (break on sector) command now accepts expressions.
   * Debugger: Added .ide_rdsec and .ide_wrsec commands.
   * Debugger: Added .diskdumpsec command.
   * Debugger: Added deref signed word (dsw) operator.
   * Debugger: .tracecio gives more detailed output.
   * Disk: Added option to format new disks with a DOS 2 filesystem.
   * Disk: Folders can now be mounted as virtual DOS 2 disk images.
   * MyIDE II emulation support.
   * Ultimate1MB emulation support.
   * Save states: Initial support for external save state files.
   * SIDE: Implemented top cartridge register and un-reversed image order.
   * Input: Input map editor now shows more specific button and axis labels.
   * IDE: Selecting a physical disk now updates the CHS mapping.
   * IDE: Fixed-size and dynamic .VHD images can now be mounted.
   * HLE/LLE: Added boot screen.
   
   ### bugs fixed
   * ANTIC: Fixed regression with the first byte in a wide scanline occasionally being decoded before being fetched.
   * ANTIC: Fixed display list pointer corruption when a jump instruction executes across vertical blank (fixes Spindizzy load).
   * Serial: Fixed R: not going into concurrent mode on open with AUX1 bit 0 set.
   * Cartridge: Re-added missing 512K detection size for SIC! cartridges.
   * Cartridge: 5200 4K/8K/16K cartridges now mirror to the full 32K region.
   * Display: Fixed offset on display in sharp bilinear mode on downlevel graphics cards.
   * Debugger: Fixed debug reads not seeing some partially overlaid memory layers.
   * Debugger: Fixed memory leak when an error occurs during expression parsing.
   * Debugger: Fixed 'value' operator returning incorrect value when used in a range write breakpoint conditional expression.
   * Debugger: Debug display now displays IR modes 8-10 correctly.
   * Debugger: Fixed history pane bugs with stack wrapping and with the scroll bar being flaky after a clear.
   * Disk: Fixed writes being gated by rotational timing even if accurate sector timing was turned off.
   * Disk Explorer: Fixed file deletion on DOS 2 volumes to not leave orphaned directory entries.
   * Disk Explorer: DOS 2 validator now allows both DOS 2 719 sector and MyDOS 720 sector VTOCs.
   * Disk Explorer: Fixed crash with large MyDOS file systems.
   * Display: Fixed text highlighting in "OS Display" overscan mode.
   * HDevice: Get Bytes command now returns $03 status code on impending EOF (fixes BLOAD from Turbo Basic XL).
   * HLE: Fixed off-by-one handling of RMARGN in screen editor.
   * HLE: Fixed some errors in updating LOGCOL on vertical cursor movement.
   * HLE: Fixed crash when disk boot sector signals a boot error (C=1).
   * HLE: Added missing international font.
   * HLE: Fixed many compatibility issues.
   * Input: Extended paddle and tablet range to 0-228.
   * Input: Tablet stylus is now always touching the tablet when the raise stylus input (button 4) is unbound.
   * Input: The "warm reset" input of the console controller was issuing a cold reset.
   * LLE: Improved display editor.
   * LLE: Fixed many compatibility issues.
   * Flash: Multiple sector erase commands are now supported.
   * Flash: Fixed Am29F040B flash to only validate A0-A10 for commands (the Am29F040 is the one that does A0-A14).
   * POKEY: Attempting to use synchronous clocking mode during serial input now produces framing errors.
   * UI: Tabs now paste properly.
   * UI: Non-ASCII characters are now saved correctly into the Altirra.ini file in portable mode.

## Version 2.10 [August 13, 2012]:
   ### features added
   * SlightSID emulation support.
   * Covox emulation support.
   * 1200XL emulation support.
   * BASIC programs can now be booted.
   * Fast boot option now accelerates timeouts for powered down disk drives.
   * Disk: Added support for format skewed ($66) command.
   * Disk: Format commands ($21, $22) now support high-speed operation.
   * Disk: Added emulation profile modes for various disk drives.
   * Disk: Reduced ACK-to-complete delay when accurate sector timing is off.
   * Disk: Added rotation and seek sound emulation.
   * Disk: Accurate disk timing prediction is now preserved when burst I/O transfers are enabled.
   * Debugger: Added .sum (sum memory area) command.
   * Debugger: Added .warmstart command.
   * Debugger: Added ap (alias pattern) command for adding aliases with patterns.
   * Debugger: Added additional aliases to a8 command (set Atari800-compatible command aliases).
   * Debugger: Added alternate L>addr syntax for setting an address range length based on an end address.
   * Debugger: Added heat map support.
   * Debugger: Added .basic_dumpline command.
   * Debugger: Enter now repeats the last command.
   * Debugger: .dumpdlist command now collapses runs of identical commands.
   * Debugger: Unassemble and dump commands now support continuation from the last address.
   * Debugger: Added deref signed byte (dsb) expression operator.
   * Debugger: Added %c and %y formatting specifiers to .printf command.
   * Debugger: Added .dmamap command to show ANTIC DMA pattern over the course of a frame.
   * Serial: 1030 Modem support.
   * Display: Added scanline display mode.
   * Display: Added integer-multiple version of preserve aspect ratio sizing mode.
   * Display: Added sharp bilinear filter mode.
   * Display: Text can now be copied out of ANTIC modes 2, 3, 6, and 7 (GR.0, 1, and 2).
   * Display: Added CIO intercept based enhanced text mode.
   * HDevice: Added minimal support for SDX Get Current Directory command ($30).
   * HDevice: Added support for H6-H9: text translation modes.
   * HDevice: Implemented rename, get/set/create/remove directory, and get file length commands.
   * HDevice/PCLink: Fixed lock command not setting read-only flag.
   * Cartridge: Added MicroCalc 32K cartridge type support.
   * Cartridge: Added cartridge type autodetection.
   * UI: Screenshot command now adjusts for interlacing and 14MHz dot clock rendering.
   * UI: The program is now marked as high DPI aware in the application manifest.
   * Simulator: Added support for MapRAM MMU modification.
   
   ### bugs fixed
   * Increased paste keyboard timer to reduce dropped keys on line processing delays.
   * Fixed startup crash on Sandy Bridge CPUs on Windows XP.
   * Simulator: Inhibit FP math acceleration hooks when PBI ROM overlay is active.
   * Simulator: BASIC disable Option key hold is now released more quickly to reduce cartridge conflicts.
   * Debugger: "read" and "write" terms can now appear more than twice in a conditional breakpoint expression.
   * Debugger: Fixed .tracecio off not working.
   * Debugger: Fixed execution stopping on TRACE statements.
   * Debugger: Breakpoint clear (bc) command no longer interprets breakpoint numbers as hex.
   * Debugger: Fixed console entry window not always disabling properly.
   * Debugger: .pokey command now reports correct status for secondary POKEY.
   * Display: Fixed crash on some XP systems on display mode change.
   * Display: Added workaround for ps1.1 precision issue on Intel graphics cards.
   * Display: Fixed bicubic scaling in OpenGL mode.
   * ANTIC: Fixed line buffer addressing to correctly handle aliasing of buffered data to different modes.
   * POKEY: Fixed glitching in high-pass filtering.
   * POKEY: Fixed wrong bit in SKSTAT being set for serial input overruns.
   * POKEY: Fixed asynchronous receive mode not resetting timers 3+4 that are already running.
   * Disk: Fixed handling of XF551 high-speed sector skew format commands ($A1/$A2).
   * Disk: Format command now sends back ACK+ERROR+buffer instead of NAK on a read-only disk.
   * Disk: Adjusted step rate of 810 drive to 5.3ms.
   * Disk: Fixed forced sector ordering command (.diskorder) for .pro images.
   * Disk: Step delays were not always reflected correctly in command delay time (only rotational position).
   * Disk: Fixed inverted write protect bit status.
   * Disk: Attempted writes to a write protected disk now return errors instead of a NAK.
   * Disk: Fixed command response when accessing a drive with no disk.
   * Disk: Fixed error return for unsupported Write PERCOM Block requests.
   * Disk Explorer: Fixed case sensitivity issue allowing duplicate file creation on DOS 2 file systems.
   * Input: Fixed bug in keyboard repeat detection code.
   * Input: Improved Caps Lock key support.
   * Cartridge: An error is now thrown when attempting to save a cartridge image in .CAR format with an unsupported mapper.
   * Cartridge: Fixed switchable XEGS cartridge modes.
   * Display: Fixed display mode matching when /f is specified on the command line.
   * Display: Improved behavior of interlaced mode when frames are dropped.
   * CPU: Fixed cycle behavior of ASL/LSR/ROL/ROR abs,X instructions in 65C02 mode.
   * HLE Kernel: Fixed BUFADR being overwritten by CIO, which broke the DOS XE executable loader.
   * IDE: Fixed banking in KMK/JZ V1 mode and added 3K ROM support.
   * IDE: Fixed multiple sector read requests of exactly 32 sectors.
   * VBXE: Fixed memory not being cleared on cold reset with randomize function disabled.
   * Serial: Fixed incompatibility with Ice-T XE and the R: handler's STATUS handler.
   * Misc: Fixed upper 4 bits for some R-Time 8 register reads.
   * HDevice: Fixed NOTE and POINT commands.
   * Cheats: Fixed 16-bit option not being tracked properly for manually entered cheats.
   * GTIA: Sprites are now triggered properly from within horizontal blank.
   * GTIA: Fixed sprite image overlap result.
   * Cassette: Loading a new tape didn't press 'play' on the emulated player until the next cold reset.

## Version 2.00 [December 24, 2011]:
   [breaking changes]
   * Debugger: The step over command has been changed from s to o.

   ### features added
   * UI: Added OS Screen size setting.
   * UI: Added support for gzip compressed images (*.gz, *.atz).
   * UI: Arrow key behavior is now configurable.
   * UI: Added speed options.
   * UI: Added option to control whether emulation sees the Shift key pressed after a Shift+F5 cold reset.
   * UI: Added MRU list for booted images.
   * UI: Color settings can now be exported as a palette file (*.pal).
   * UI: Display API options can now be changed through the GUI (not just cmdline).
   * UI: Added full screen resolution options.
   * UI: Added single instance option.
   * UI: Request 1ms system timer precision only when unpaused.
   * UI: Added option to bind file associations in Windows for image types.
   * UI: Added /run, /disk, /tape, and /cart switches to force load types on the command line.
   * UI: Added mute option.
   * Debugger: Added command aliases (ac, al, as, a8).
   * Debugger: Added watch expression (wx) command.
   * Debugger: Added multiple memory panes.
   * Debugger: Added watch panes.
   * Debugger: Initial version of debug display.
   * Debugger: Disassembler can now display offsets from symbols.
   * Debugger: Profiler now distinguishes between VBIs and DLIs in function sampling mode.
   * Debugger: Profiler now shows unhalted cycle counts and DMA contention rates.
   * Debugger: Added call graph profiling support.
   * Debugger: Mouse wheel is now position-based instead of focus-based.
   * Debugger: Address range breakpoints can now trigger commands.
   * Debugger: Added log output filter commands (lfd/lfe/lfl/lft).
   * Debugger: Added .ide command.
   * Debugger: Enter (e) command now takes expressions.
   * Debugger: Added bitwise/low-byte/high-byte operators and address/value variables to expression evaluator.
   * Debugger: Break on expression command (bx) can now include address ranges.
   * Debugger: Added dump double words (dd) command.
   * Debugger: Go (g) command now has options to preserve and force source mode.
   * Debugger: Added batch (.batch) command.
   * Debugger: Added source mode control (.sourcemode) command.
   * Debugger: Commands can now be queued to run when an EXE loads or runs.
   * Debugger: Source line breakpoints can now be set prior to line debug information being available.
   * Debugger: A matching .atdbg file is now processed when an executable loads.
   * Debugger: startup.atdbg in the program directory is now processed on startup if present.
   * Debugger: Ctrl+Up/Down from output window command line now scrolls log pane by single lines.
   * Debugger: Added fill (f), search (s), and move (m) commands.
   * Debugger: Register (r) command can now set individual flags.
   * Debugger: Improved output of .pia command.
   * Debugger: Call, loop, and interrupt collapsing in history window can now be toggled.
   * Debugger: History window can now show timestamps in cycles, unhalted cycles, and microseconds from an anchor point.
   * Debugger: Extended memory syntax now allows access to hidden RAM (r: prefix).
   * Debugger: Added .tape and .tapedata commands.
   * Disk: Added support for Write PERCOM Block command.
   * Disk: Added option to auto-detect firmware overrides of disk accesses and automatically disable SIO patch for those drives.
   * Disk: Live disk images can now be explored directly from the disk drive entry rather than requiring an unmount and remount.
   * Disk: Disk explorer shows timestamps.
   * Disk: .ARC compressed archives can now be explored.
   * Disk: Fixed a couple of bugs with DCM disk image decoding.
   * Cartridge: Added another Atrax 128K cartridge variant used for SpartaDOS X.
   * Cartridge: Added support for OSS 8K, Blizzard 4K, AST 32K, Atrax SDX 64K, Turbosoft 64K, and Turbosoft 128K types.
   * Cartridge: Added support for newer MaxFlash 1MB cartridges that power up in bank 0 instead of bank 127.
   * Cartridge: Added support for 1M Megacart (different type with similar name).
   * Cartridge: Added support for a 5200 64K cartridge with 32K banks.
   * Input: MultiJoy8 support.
   * Input: CX-80 and 5200 trackball support.
   * Input: 1200XL F1-F4 keys can now be enabled in keyboard options.
   * Input: ~ key is now an alternate mapping for the inverse key.
   * IDE: KMK/JZ IDE / IDEPlus 2.0 emulation support.
   * IDE: Initial SIDE emulation support.
   * IDE: Solid-state mode supports up through PIO mode 6.
   * IDE: Read-only physical disk access.
   * GTIA: SECAM support.
   * GTIA: Optimizations to mode 9 and mode 10 rendering.
   * MMU: Added 800 8K/24K/32K/40K configurations.
   * MMU: Added 320K and 576K Compy Shop configurations.
   * MMU: Sped up extended bank switching.
   * Simulator: Added XEGS support.
   * SoundBoard: Multiplier support.
   * POKEY: Rewrote sound core to defer repeating events whenever possible for extra speed.

   ### bugs fixed
   * ANTIC: Added emulation of bus data displaying at the very right border of a wide scrolled playfield.
   * ANTIC: Improved accuracy of mid scan line changes to HSCROL and DMACTL.
   * ANTIC: Fixed phantom DMA data during WSYNC.
   * ANTIC: Implemented phantom DMA artifacts along right side of wide playfield.
   * ANTIC: Fixed NMIs not being reactivated after a write to NMIRES on cycle 7.
   * Disk: PERCOM block is now initialized for a drive without a disk.
   * Disk: Fixed errors in PERCOM block track and sectors per track counts.
   * Disk: Switching a disk from R/W to VirtRW mode now clears pending flush errors.
   * Disk: Fixed incorrect serial transfer rates for XF551 high speed Put/Write and Write PERCOM commands.
   * Disk: Fixed H: device not handling zero-byte get/puts properly with burst I/O enabled (affected GET/PUT from BASIC).
   * Disk: H: device now supports OPEN with AUX1=7 to fix directory listing from DOS 2.5.
   * Disk: Disk explorer now properly increments volume sequence number when modifying SpartaDOS X images.
   * Disk: DSKINV hook now supports writes to sectors bigger than 128 bytes.
   * Disk: Fixed crash with very short disk images.
   * CPU: Fixed undocumented $BB opcode using abs instead of abs,Y addressing mode.
   * CPU: Undocumented $6B opcode now supports decimal mode.
   * Debugger: Profiler sometimes lost track of intermediate call frames during function sampling.
   * Debugger: Mouse wheel now uses system wheel scroll setting.
   * Debugger: Improved parsing of MADS listings.
   * Debugger: Fixed command escaping so that escapes are no longer processed unless enabled, i.e. \"foo\nbar".
   * Debugger: Register (r) can no longer clear bits 4 and 5 of P when unsupported by current CPU mode.
   * Debugger: _ and . are now accepted as part of symbol names in expressions.
   * Debugger: Fixed crash when attempting to load empty file as symbol file.
   * Debugger: Source path matching now allows partial path matches both ways and has improved behavior with duplicate filenames in different directories.
   * Debugger: Fixed wrapping of frame counter in history window.
   * Debugger: Built-in kernel symbols now take priority over loaded symbols.
   * Debugger: Fixed page scrolling in history window after enabling CPU history tracking.
   * IDE: Added mandatory Recalibrate, Read Verify, Read Multiple, Write Multiple, Seek, Set Multiple Mode, and Initialize Device Parameters commands.
   * IDE: Fixed response from device 1 status register.
   * IDE: Improved response from Identify Drive command.
   * IDE: Set transfer mode command is now supported.
   * Cartridge: Reverted OSS 034M mapper back to original bank order and added OSS 043M mapper mode for alternate bank layout.
   * Input: Fixed crash when activating triggers to unavailable controllers.
   * PIA: Implemented IRQB and control bit 6.
   * HLE: Fixed escape handling in display handler.
   * HLE: Screen setup now matches the display list and playfield addressing of the stock OS ROM.
   * HLE: Fixed cartridge handling so that cartridges are run after disk boot completes.
   * HLE: Added known RTS instruction at $E4C0.
   * HLE: CIO now allows SPECIAL commands to be issued on a closed IOCB.
   * HLE: Boot loader now sets DBYTLO/DBYTHI to $0400 before launching.
   * HLE: Fixed crash in CIO on OPEN command with unknown device.
   * HLE: S: now supports the Get Status, line, and fill commands.
   * UI: Disk sector display was incorrect for accelerated writes.
   * UI: Fixed crash if DirectX 9 is not installed.
   * UI: Display window can no longer be undocked in full screen mode.
   * UI: Adjusted default NTSC color preset. The old preset is still available.
   * UI: Command-line processing now better matches standard VC++ escaping rules.
   * UI: Direct3D9/OpenGL rendering now works over Remote Desktop with Windows Vista/7.
   * SoundBoard: Restricted $D5/D6xx mapping to $D500-D53F and $D600-D63F.
   * POKEY: Fixed incorrect latching on IRQST bit 4 (serial output ready IRQ).
   * POKEY: Fixed deassert timing for IRQST bit 3 (serial output complete IRQ).
   * POKEY: Added partial emulation of SKSTAT bit 4 (direct input) during disk transfers.
   * Cassette: Fixed data blocks being silently dropped when reading some .cas files.
   * Cassette: Fixed inaccuracy in data rate produced from loading .cas files.
   * GTIA: Fixed incorrect color for %1000 pattern in mode 10 with lores ANTIC input.
   * VBXE: Warm reset no longer resets all registers like a cold reset.

## Version 1.9 [May 15, 2011]:
   ### features added
   * Debugger: Multiple memory access breakpoints can now be placed.
   * Debugger: Added option to break on run address of loaded executable.
   * Debugger: Conditional breakpoints (bx) are now supported.
   * Debugger: Commands can now be run when a breakpoint fires.
   * Debugger: Added ? (evaluate) and .printf commands.
   * Debugger: Trace and assert statements can now be embedded in assembler listings.
   * Debugger: Added vta, vtc, vtl, and vtr commands to control verifier OS entry point checking.
   * Debugger: Added .dumpsnap command.
   * Debugger: Memory access breakpoints can now be placed on an address range.
   * Debugger: History window now has a Copy Visible context menu option.
   * Debugger: Verifier modes are now configurable.
   * Debugger: Verifier can now catch register corruption across interrupts.
   * Debugger: .writemem now supports extended address syntax.
   * Debugger: Added .sio command.
   * Debugger: Added .sdx_loadsyms command.
   * SoundBoard: Initial support.
   * UI: Added emulator crash dialog.
   * UI: Show FPS option now also shows CPU usage.
   * Input: Added Amiga mouse support.
   * Input: Added CX-85 keypad support.
   * GTIA: NTSC and PAL modes can now have separate palettes.
   * GTIA: Added option for PAL encoding quirks.
   * GTIA: Added PAL high artifacting mode.
   * Disk: Initial version of disk explorer.
   * Serial: Inbound connections are now supported.
   * Serial: Added support for A, L, M, P, Q, S, T, V, X, &C, &D, &F, &G, &P, &T, &V, and &W modem commands.
   * Serial: Added DTR and CRX signal support.
   * Serial: Added IPv6 support.
   * Serial: Added option to force specific connection and DTE communication rates.
   * Serial: Added option to bypass accurate serial port rate throttling.
   * Cartridge: Two cartridges are now supported for 800 left+right and stacked cartridge configurations.
   * Cartridge: 2K and 4K cartridges are now supported (as mirrored 8K cartridges).
   * Cartridge: SIC! cartridge support.
   * PCLink: Initial support.
   
   ### bugs fixed
   * UI: Pane dragging is less twitchy and no longer undocks a pane as soon as you click on the caption.
   * UI: Fixed undocked panes getting lost on exit when no other panes were docked.
   * UI: Optimized Direct3D9 code to reduce flickering when switching artifacting modes.
   * UI: Fixed current beam position indicator not updating properly when stepping with interlacing enabled.
   * UI: Auto-size command now works when menu wraps.
   * HLE: Fixed crash when NMI handler is invoked with no NMIs pending.
   * HLE: Added support for XL OS PENTV vector.
   * HLE: E: device now excludes prompts from data returned by get-line commands.
   * HLE: Increased HLE kernel compatibility with SpartaDOS X.
   * HLE: Fixed BASIC ^ operator with HLE kernel or floating-point acceleration.
   * HLE: Fixed AFP not accepting leading spaces (which broke the Atari BASIC READ statement).
   * Simulator: Default kernel mode now prefers ROM images properly again.
   * Simulator: BOOT? and DOSINI are now set at the beginning of EXE/XEX loads to support more WARMSV tricks.
   * Cheats: Enable checkbox on cheats was broken.
   * CPU: Added missing SHA abs,Y ($9F) instruction.
   * CPU: Fixed CMOS BIT #imm ($89) instruction.
   * CPU: Fixed 65C02 $FC instruction.
   * CPU: Back-to-back interrupts are no longer allowed.
   * CPU: Added emulation of BRK/NMI bug.
   * CPU: Added option to emulate masked NMIs (off by default).
   * CPU: Fixed dp,X, dp,Y, and (dp,X) page wrapping behavior in 65C816 E=1 mode.
   * ANTIC: DMACTL bits 6 and 7 are no longer reflected in .dlhistory output.
   * ANTIC: Fixed CPU not being halted when playfield DMA occurred on cycle 10.
   * ANTIC: Fixed timing of when VSCROL affects DLIs.
   * Debugger: Optimized History pane.
   * Debugger: Avoid re-requesting the same source file when it is not found on disk.
   * Debugger: Improved reliability of source-level stepping.
   * Debugger: Added XL OS entry points to default entry points allowed by verifier.
   * Recorder: Fixed crash on recording errors.
   * Recorder: Fixed incorrect format on video stream when using ZMBV compression.
   * Disk: SIO receive speeds are now validated for more reliable high-speed SIO detection.
   * Disk: VAPI (ATX) images can now be loaded from .zip files.
   * Disk: Fixed problems binding disk images to D9:-D15:.
   * GTIA: NTSC high artifacting mode now respects artifact phase setting.
   * GTIA: Fixed trigger state sometimes being initially wrong when enabling latching.
   * GTIA: Improved accuracy of transitions from mode 8 to mode 9/10.
   * GTIA: Fixed hires lines not artifacting correctly with the extended PAL height setting enabled.
   * Serial: Telnet emulation now reads out of band data.
   * Serial: Telnet emulation now handles incoming WILL and WONT commands.
   * Serial: Telnet server now attempts to turn off local echo on remote end.
   * Serial: Fixed sporadic missing newlines on modem status messages.
   * Serial: Control line state is now reported in non-concurrent mode.
   * Serial: Ring pattern adjusted to match US standard (2s/4s).
   * Serial: Re-opening the R: device now flushes buffered data.
   * Serial: Closing the R: device deactivates concurrent mode to make subsequent status requests work.
   * Serial: Fixed crash in R: when invoking GET BYTE with an empty input buffer or PUT BYTE with a full output buffer.
   * Serial: Fixed BASIC GET command failing after XIO on R:.
   * Serial: Fixed erroneous CR-to-CRLF conversion on transmit path.
   * Serial: XIO commands to R: instead of R1-R4: now work properly.
   * Serial: Closing R: waits for queued output data to flush.
   * Printer: Fixed close command not actually closing IOCB.
   * VBXE: VBXE memory no longer overlays extended memory in 1088K mode with shared memory disabled.
   * IDE: Cold reset now also resets the emulated IDE device.
   * IDE: Addressing is now restricted to offsets $00-07 to avoid conflicting with OS PBI scan.
   * Cartridge: Corrected banks 1 and 2 for OSS 034M cartridge type.

## Version 1.8 [December 22, 2010]:
   ### features added
   * Initial 5200 support.
   * Simulator: Added 16K memory size support.
   * Simulator: 576K and 1088K modes now allow ANTIC access to extended memory.
   * Simulator: ROM image paths are now configurable.
   * Cartridge: Added support for Corina 1MB + 8K EEPROM and 512K + 512K SRAM + 8K EEPROM cartridges.
   * Cartridge: Added support for Telelink II NVSRAM.
   * Cartridge: Added support for DB 32K, Williams 32K / 64K, Diamond 64K, SpartaDOS X 64K/128K, Phoenix 8K, Blizzard 16K, and Atrax 128K cartridges.
   * Cartridge: Added support for 8K right slot cartridges.
   * Cartridge: Added support for non-switchable 256K-1M XEGS cartridges.
   * Cartridge: Mapper dialog attempts to detect Atari 2600 cartridge images (unsupported).
   * Debugger: The history window no longer needs to be closed and reopened after toggling history mode.
   * Debugger: Source windows now highlight lines that have symbol information associated and can be used for execution control.
   * Debugger: Added on-screen watch byte/word commands (wb/ww/wc/wl).
   * Debugger: History window now detects and collapses loops.
   * Debugger: Added optional length parameter for dump and disassemble commands.
   * Debugger: Added dump INTERNAL string command (di).
   * Debugger: Added .iocb command.
   * Debugger: Added ya/yc/yd/yr/yw commands for manually adding symbols.
   * Debugger: Verifier now detects recursive NMIs.
   * Debugger: Font and font size can now be changed.
   * Debugger: Added 6502 miniassembler.
   * Debugger: Added detailed command help.
   * Debugger: Added .pathrecord command and documented existing path commands.
   * Debugger: Added .basic and .basic_vars commands.
   * Debugger: Console window now supports multiple history entries and page up/down from the command line.
   * Debugger: Added symbol parsing support for ATasm and XASM assembler output.
   * Disk: Added support for D9: through D15:.
   * UI: Added DirectSound support.
   * UI: Audio latency can now be adjusted.
   * UI: Switched to larger font for on-screen UI.
   * UI: Shift key now controls whether dropped files are mounted/added (shift) or booted (no shift).
   * UI: Left-shift and right-shift keys can now be bound separately.
   * UI: Warnings are now displayed before modified images are implicitly discarded.
   * UI: Enabled themed controls.
   * UI: Removed borders on docked panes.
   * UI: Added first pass at pause button (F9).
   * UI: Ctrl+Arrow keys now give the un-Ctrl'd keys on the emulated keyboard for programs that use pseudo arrow keys.
   * UI: Mouse auto-capture is now an option.
   * UI: Added option to resize main window to remove letterboxing on display.
   * Input: Input map editor redesigned for easier controller configuration.
   * Input: Input maps now support shifted subsets via flags.
   * Input: Analog axes can now be bound as either relative or absolute, with tunable speed for relative mode.
   * Input: Paddles now support rotation-based control.
   * Input: Added light pen/gun support.
   * Input: Added tablet support.
   * VBXE: 5200 support added.
   * IDE: Added option to switch between short and long BSY-to-DRQ delays to simulate platter and solid-state media.
   * Serial: Added support for 850 R: and basic virtual-modem-over-TCP emulation.
   * Serial: Added stub loader emulation of 850 SIO boot process.
   * Cheats: Added simple cheat finder and value locker.
   * Recording: Added options to tweak the frame rate and to record at half rate.

   ### bugs fixed
   * Disk: Added workaround for yet another goofy way to store boot sectors in a DD ATR image (3 x 128K followed by 384K of nulls).
   * Disk: Fixed crash when opening .zip file containing files with no filename extension.
   * Disk: Disk drives dialog now highlights disk images that have been modified and not saved.
   * IDE: Fixed excessively long delay for BSY to assert after a command is issued.
   * Input: Binding mouse up/down/left/right outputs to digital inputs now works.
   * UI: Fix for crash when using Direct3D display on video cards that don't support arbitrary non-pow2 textures.
   * UI: Rewrote audio output path to use block duplication/removal rather than resampling to maintain sync.
   * UI: Fixed emulated mouse button not working reliably.
   * UI: Reduced border artifacts when using bicubic display stretching.
   * UI: Fixed crosstalk between interlace and frame blending settings.
   * PIA: Control bits 6 and 7 are no longer writable.
   * POKEY: Fixed RANDOM offset in 17-bit noise generator mode.
   * POKEY: Fixed IRQ timing for CPU-triggered IRQs.
   * POKEY: Fixed serial output complete interrupt not always deactivating properly.
   * POKEY: Clearing SKCTL bit 1 now disables the keyboard.
   * POKEY: Adjusted timing of 15KHz and 64KHz timers after exiting initializing mode.
   * GTIA: Phantom P/M graphics DMA is now implemented.
   * GTIA: Fixed graphic output when changing the size in the middle of a player image.
   * GTIA: Improved accuracy of graphic output when changing GTIA modes mid-screen.
   * GTIA: Latched triggers are now implemented.
   * HLE: Delete line ($9C) now resets cursor column.
   * HLE: Implemented CRSINH function.
   * HLE: IRQ handler now clears decimal flag.
   * HLE: Fixed crash when jumping to cassette init routine.
   * Debugger: Fixed MADS listing parsing for code lines after ORG statements.
   * Debugger: .dumpdsm command works again.
   * Debugger: History window now shows effective addresses.
   * Debugger: Disassembly and history windows now allow disassembly columns to be toggled.
   * Debugger: Stepping in source view now steps by source line instead of instruction.
   * Debugger: Fixed uninitialized screen data sometimes appearing when stepping.
   * Debugger: Register "r" command can now set A register.
   * Debugger: Breakpoints now work on the run address of a loaded executable.
   * Debugger: Fixed .writemem crashing if filename was invalid.
   * CPU: Fixed IRQ timing around CLI and SEI instructions.
   * CPU: Fixed a couple of bugs with decimal mode.
   * CPU: Taken branches now delay IRQ/NMI acknowledgement by one cycle.
   * CPU: Fixed false read behavior for: STA (zp),Y; STA abs,Y; STA abs,X; STX abs,Y.
   * CPU: Implemented reserved NOPs in 65C02 mode.
   * CPU: Fixes to undocumented opcodes: RLA ($23, $27, $2F, $33, $37, $3B, $3F); ASR ($4B); SAX ($83, $87, $8F, $97); SHX ($9E).
   * ANTIC: Fixed occasional unexpected DLIs on cold reset, which was causing once-only emulation crashes.
   * ANTIC: Corrected NMIST change timing.
   * Cartridge: Fixed OSS 034M and M091 mappers.
   * Recording: Fixed crashes in some cases when recording start process failed.
   * VBXE: Attribute map RES bit is now implemented.
   * VBXE: Fixed attribute map addressing getting scrambled on mid-screen changes.
   * VBXE: Overlay scrolling and addressing is now reset at VBL and no longer affects bitmap modes.
   * VBXE: Frame blending is now supported in VBXE mode.

## Version 1.7 [September 4, 2010]:
   ### features added
   * Cartridge: Fixed ID mistake on mappers 1 and 2; added 3, 15, 26-30, 32-38.
   * Cartridge: Added dialog to ask for mapper when header is missing.
   * Cartridge: Added write support for 1Mb/8Mb flash cartridges.
   * Cartridge: Added option to create new 1Mb/8Mb flash cartridges.
   * Cartridge: Added option to save cartridge images.
   * Cartridge: Added support for AtariMax 1Mb+MyIDE cartridge banking mapper.
   * UI: Added /bootvrw and /bootrw to boot disks in read/write or virtual R/W mode.
   * UI: Added /artifact switch for controlling video artifacting.
   * UI: Multiple types of images can now be mounted simultaneously via the command line.
   * UI: Added /type switch to push keys on startup.
   * UI: Added /cartmapper switch to specify mapper when booting headerless cartridges.
   * UI: Added support for saving settings in an .ini file instead of the Registry.
   * UI: Added more color presets.
   * UI: Added save screenshot command.
   * UI: Ctrl+\ and Ctrl+Shift+\ now produce Ctrl+Esc and Ctrl+Shift+Esc.
   * UI: Added /portable mode to switch to Altirra.ini for settings.
   * UI: Added help file.
   * UI: Disk, cartridge, and tape images can now be loaded from .zip files.
   * UI: Heads-up display now indicates H: access.
   * Recording: Added video recording.
   * Simulator: Added option for faster kernel boot by accelerating checksum and RAM test routines.
   * Simulator: .bin and .xfd extensions now override content-based identification for images.
   * Simulator: BASIC disable now lifts Option key on playfield enable rather than VBI enable to avoid breaking games that don't use VBIs.
   * Debugger: Added .pathbreak command to stop when new code is executed for the first time.
   * Debugger: Disassembly window now auto-pages when scrolling up or down beyond the current view.
   * Disk: Individual disk drives can now be powered on and off from the Disk Drives dialog.
   * Disk: Improved compatibility of H: device.
   * Disk: Added support for H: burst I/O.
   * IDE: Added emulation support for MyIDE interface.
   * HLE: Rearranged SIO to improve compatibility with naughty programs that copy out parts of kernel ROM.

   ### bugs fixed
   * UI: Fixed random crash when opening debugger or new debug panes.
   * UI: Booting images on the command line now removes previously mounted images.
   * UI: Fixed Alt+8 keyboard shortcut.
   * UI: Remapped Insert/Delete/Home keys to better match their Atari equivalents.
   * UI: Fixed joystick ports 3 and 4 not being mappable.
   * UI: Fixed focus anomalies when bringing up profile view undocked.
   * UI: Rewrote bicubic pixel shader to fix artifacts on Intel GMA 950.
   * UI: Display acceleration is now supported on secondary monitors.
   * POKEY: Added support for using channel 2 as serial transmit clock (SKCTL[6:5]=11).
   * POKEY: Unassigned registers now properly read $FF.
   * Disk: Increased delay from complete byte to first data in high speed mode to fix data overruns in SIO-based routines.
   * Disk: Fixed bug that caused corruption of newly created disk images during incremental updates.
   * CPU: Fixed bug in DCP opcodes.
   * CPU: Added missing ARR #imm ($6B) opcode.
   * CPU: Added missing SAX zp,Y ($97) opcode.
   * CPU: Added missing XAS abs,Y ($9B) opcode.
   * CPU: Fixed ATX #imm ($AB) opcode.
   * Debugger: Fixed crash/errors when trying to reopen printer pane after closing it.
   * Debugger: Memory leak fixes.
   * Debugger: Fixed disassembly window jumping when changing breakpoints after selecting a nested call stack frame.
   * Debugger: Fixed Alt+Shift+0 shortcut for profiler view.
   * Simulator: Fixed bug where BASIC bit wasn't handled properly in 576K and 1088K memory modes.
   * Cartridge: SuperCharger 3D cartridge is now saved and restored properly across runs.
   * HLE: Added initial cassette reading support.
   * HLE: Fixed bugs in E: logical line handling in bottom third of screen.
   * HLE: Fixed handling of undocumented ZFL entry point in math pack with accelerated FP disabled (improves compatibility with Atari BASIC).
   * HLE: SIO now disables level 2 VBLANK when active.
   * HLE: CIO no longer throws an error when closing an IOCB that is not open.
   * HLE: CIO now allows opening a device without a colon, i.e. "K".
   * GTIA: Fixed some display artifacts when extended PAL height is enabled.

## Version 1.6 [April 25, 2010]:
   ### features added
   * Debugger: Added profiler.
   * Debugger: Added support for loading CA65 labels.
   * Debugger: Added verifier to catch possible coding errors.
   * Debugger: Added support to view extended memory to some commands and panes.
   * UI: Added option for controlling vertical sync lock.
   * UI: Added /opengl command line switch.
   * UI: BASIC ROM is now noted on window caption.
   * UI: Window position is saved on exit.
   * UI: Added /debug switch to launch in debugger mode.
   * UI: Added /f switch for full-screen mode.
   * UI: Added .car extension to Open Image dialog.
   * UI: Input maps can now be cloned.
   * UI: Input maps can now be bound to a specific game controller index.
   * UI: Create disk dialog now has entries for standard disk formats.
   * UI: Added ability to paste text from clipboard.
   * UI: Moved warm reset to F5, bound Help to F6 and Break to F7, and merged Run into F8 to match other emulators.
   * UI: Added program icon.
   * Simulator: Added option to fill memory with a psuedorandom pattern on cold boot instead of zeroes.
   * Simulator: Added support for 65C816 high memory at banks $01-03.
   * POKEY: Increased playback sampling rate to 44KHz.
   * POKEY: Added option to toggle non-linear mixing.
   * POKEY: Added option to toggle individual sound channels.
   * POKEY: Implemented keyboard overrun bit.
   * VBXE: Register window can now be switched between $D6xx and $D7xx.
   * Disk: Added support for 1050 drive dual density format.
   * Disk: Added support for XF551 drive high speed commands.
   * GTIA: Added a stronger level of NTSC artifact emulation.

   ### bugs fixed
   * Fixed command line parsing and added several new switches.
   * HLE: Fixed math pack issues preventing Atari Basic ATN(), INT(), and SQR() from working.
   * HLE: Fixed handling of BASIC enable/disable on warm reset.
   * LLE: Improved math pack implementation.
   * ANTIC: Playfield DMA wasn't properly shut off on cold reset.
   * ANTIC: Improved emulation of mid scan line DMACTL changes.
   * ANTIC: Reset line now resets position counters.
   * ANTIC: Emulated single cycle glitch in VCOUNT at the end of a frame.
   * ANTIC: Implemented delay for changes to CHBASE to take effect.
   * POKEY: Adjusted timing of STIMER and timer rollovers.
   * POKEY: Retuned filters and non-linear mixing curve.
   * POKEY: Fixed behavior of SKSTAT serial overrun bit.
   * GTIA: Added option for displaying full PAL height when extending display past scan line 239.
   * GTIA: Fixed display of beam position when single stepping with VBXE enabled.
   * Debugger: Fixed bug where history window didn't allow scrolling to the bottom via the slider.
   * Debugger: History window now always shows last executed instruction.
   * Debugger: History window now flushes and rebuilds the tree when the history window rolls over.
   * Debugger: "Toggle read/write breakpoint" in the memory window was using the wrong address.
   * Debugger: Disassembly window now tracks 65C816 M/X/E state.
   * Debugger: .history command was using wrong register state for decoding indexed effective addresses.
   * Debugger: Fixed incorrect index register values in Registers pane in 65C816 emulation mode.
   * Debugger: Fixed disassembly of JMP (abs,X) and JSR (abs,X) instructions.
   * CPU: XCE instruction no longer sets M/X bits when staying in native mode.
   * CPU: Fixed bank used by JML, JMP (abs), and JMP (abs,X) instructions in 65C816 mode.
   * Disk: Added support for reading broken DD disks that have full 256 byte boot sectors.
   * Disk: SIO acceleration hook now sets SKCTL correctly.
   * UI: Fixed issue where full-screen mode didn't always cover the entire screen.
   * Simulator: Shortened delay to releasing OPTION on startup.

## Version 1.5 [January 31, 2010]:
   ### features added
   * Added option to redirect P: to a text pane.
   * UI: Added handing for gamepads being added or pulled during execution.
   * UI: Rewrote input system and added general input mapper.
   * UI: Added options for controlling display stretching and aspect ratio.
   * UI: Added better handling for cases in which a requested firmware ROM is not available.
   * UI: Added option to control overscan display.
   * UI: Mounted disks and cartridges are saved and restored across runs.
   * UI: FPS display in window caption is now optional.
   * UI: System configuration is now shown on the window caption.
   * POKEY: Added volume control.
   * Simulator: Added "Other" firmware option to load an otheros.rom custom kernel (16K XL).
   * Cassette: Added seek UI.
   * Cassette: Reworked playback indicator.
   * GTIA: Added option to blend adjacent frames to reduce flicker.
   * GTIA: Added System > Video > Adjust Colors.
   * GTIA: Added support for interlaced output.
   * Disk: Added sector counter option.
   * Disk: Increased max drive count to 8.
   * Disk: Added burst transfer mode compatible with SpartaDOS X.
   * Disk: Get Status and Read PERCOM Block commands are now accelerated.
   * Disk: Lazy writes are now incremental.
   * VBXE: Initial emulation support.
   * Misc: Added R-Time 8 support.
   * Debugger: Added list nearest symbol (ln) command.
   * Debugger: "Go to Source" command now asks for source file location if it cannot be found.

   ### bugs fixed
   * HLE: Improved compatibility of screen editor.
   * HLE: Fixed bug in emulated math pack that broke Atari Basic.
   * Debugger: Fixed crash when a source file couldn't be found.
   * Debugger: Auto-loading of symbols for executable files works again.
   * Debugger: Source windows no longer recenter when toggling breakpoints.
   * Debugger: Fixed misspelled SKRES symbol.
   * ANTIC: Fixed incorrect CPU timing for STA WSYNC + PHA combination.
   * ANTIC: Vertical scroll bit is now kept across VBLANK if display list is too long.
   * ANTIC: Fixed timing of display list DMA enable changes (DMACTL bit 5).
   * ANTIC: Internal line buffer is no longer cleared when repeating mode lines with display list DMA disabled.
   * ANTIC: Mode lines can now repeat across VBLANK with display list DMA disabled.
   * ANTIC: Added emulation of bug where VBLANK can fail to occur if scan line 247 is a hires line.
   * POKEY: Fixed reading of POT timers before pot counters have stabilized.
   * CPU: Implemented SBX instruction ($CB).
   * Disk: Increased reliability of lazy flush mechanism.
   * Disk: Fixed PERCOM block for large disk images.
   * Disk: Fixed boot sector count for 512 byte/sector images.
   * Disk: Hard disk (H:) no longer goes away on a warm reset.
   * Disk: Reverted change to raise RTCLOK on SIO acceleration to fix bugs with programs that monitor RTCLOK from a VBI handler.
   * UI: Mouse capture is automatically released when the simulator stops.
   * UI: Fixed USER handle leaks.
   * Simulator: 1088K memory mode no longer disables self-test bit when extended RAM is disabled.
   * LLE: Fixed cartridge initialization.
   * LLE: Fixed BASIC ROM switching.
   * LLE: K: open/close calls are now implemented.
   * LLE: Added initial math pack implementation.
   * LLE: SIO now shuts off all audio channels.

## Version 1.4 [November 8, 2009]:
   ### features added
   * Debugger: Added .diskorder command to force phantom sector load order.
   * Debugger: Added .dma command to dump current ANTIC DMA pattern.
   * Debugger: Added .caslogdata to diagnose cassette data read failures.
   * Debugger: Added .pia command to dump PIA state.
   * Disk: Added support for command $4E (Read PERCOM Block).
   * Disk: Added support for reading Diskcomm (DCM) images.
   * Cassette: CAS images and mono WAV files now play data as audio.
   * Cassette: The SIO acceleration engine now reports diagnostics about damaged blocks to the console window.
   * Cassette: Made accelerated read routine more robust against interpreting garbage as the start of block sync.
   * Cassette: Optimized WAV processing routine.
   * Cassette: Added command to unload tape.
   * Cassette: Added emulation of feedback from SIO motor control line into audio input.
   * UI: Added command to reset window layout.
   * UI: Drag-and-drop is now a synonym for the Open Image command.
   * UI: Added more key mappings.
   * UI: Added Boot Image command to unload all existing cartridges/disks/tapes and cold reset after loading an image.
   * UI: Added debugging option to dump raw audio to disk (32-bit float @ 63920.4 NTSC / 63337.4Hz PAL).
   * UI: Added .com to list of file filters for the Open Image/Boot Image commands.
   * UI: Added enhanced text display mode.
   * Simulator: Added support for loading OS-A kernel ROM.
   * GTIA: Added support for PAL artifacting.

   ### bugs fixed
   * UI: Mapped Ctrl+alpha/num and Ctrl+Shift+alpha/num keys.
   * UI: Arrow-keys now auto-repeat.
   * UI: View > Display works when the debugger is disabled.
   * UI: Fixed window activation and sizing problems when toggling full screen mode.
   * UI: This simulation is now throttled in full-screen mode.
   * CPU: Implemented TDC instruction in 65C816 mode.
   * CPU: Implemented undocumented $93 (SHA) and $BB (LAS) opcodes.
   * CPU: Fixed flags on $9C (SHY) opcode.
   * CPU: Removed bogus 65C816 opcode ($DC) in 6502 decoder.
   * Cassette: Fixed desync between audio and data after accelerated load.
   * Cassette: Audio/data channels are no longer swapped when processing stereo input.
   * Cassette: Raised cutoff on control line low-pass filter to fix reliability problems at turbo baud rates.
   * Cassette: Fixed incorrect SIO return code for accelerated cassette reads.
   * Cartridge: Fixed MMU mapper so that the cartridge ROM now overrides BASIC at $A000-BFFF.
   * Cartridge: TRIG3 is now deasserted when $A000-BFFF is unmapped.
   * Cartridge: The second bank setting for Bounty Bob Strikes Back! cartridges is now saved in save states.
   * Disk: Added support for weak bit emulation in VAPI images.
   * Disk: Fixed VAPI image read errors when missing sectors are present.
   * Disk: Fixed incorrect FDC status when loading long sectors that set the "lost data" status bit.
   * Disk: SIOV/DSKINV acceleration handlers set more kernel variables for better compatibility.
   * Disk: Abort acceleration attempt when disk buffer contains SIO timeout flag (TIMFLG).
   * Disk: Adjusted sector read delay to increase VAPI image compatibility.
   * Disk: Fixed cases where the virtual read/write setting was not obeyed properly.
   * GTIA: Changes to GRAFPx/GRAFM are latched and no longer affect a sprite image that is already shifting out.
   * GTIA: Increased timing precision of console speaker from 28 cycles to single cycle.
   * GTIA: Fixed bug that caused some apps to misdetect the video mode as SECAM.
   * GTIA: Adjusted timing for mid-screen changes to P/M graphics and color registers.
   * GTIA: CONSOL output bit 3 wasn't affecting input bit 3.
   * GTIA: VDELAY now masks DMA fetches rather than actually delaying loads.
   * POKEY: Fixed random crash in stereo mode.
   * POKEY: 15KHz and 64KHz clocks are no longer joined between the left and right channel chips.
   * POKEY: The channels now have the correct 0-3 clock skew for reading the noise generators.
   * POKEY: Implemented channel 1/2 inversion when not in volume-only mode.
   * POKEY: Implemented non-linear mixing.
   * POKEY: Audio plays again when initialization mode is active.
   * POKEY: Implemented serial force break bit (SKCTL bit 7) affecting two-tone mode.
   * POKEY: STIMER now resets output channel states properly.
   * POKEY: Asynchronous serial input mode now holds down timers 3 and 4 again when a byte is not shifting in.
   * HLE: The math pack is now emulated even if the acceleration option is disabled.
   * HLE: POTGO is now strobed during vertical blank stage 2 processing.
   * HLE: STRIG2/3 are now updated from TRIG0/1 instead of TRIG2/3 for XL/XE hardware compatibility.
   * HLE: Fixed bug where POKEY was put into initialization mode after an unaccelerated SIO transfer.
   * HLE: POKEY audio channels are now shut off after an SIO operation.
   * HLE: Added keyboard click.
   * ANTIC: Two-line resolution mode now reloads GTIA every scan line.
   * ANTIC: Fixed timing when using read-modify-write instructions to trigger WSYNC.
   * Simulator: Memory banking wasn't updated properly on changes to the port B data direction register.

## Version 1.3 [September 4, 2009]:
   ### features added
   * Debugger: Register pane now shows 65C816 state.
   * Debugger: Added context menu to disassembly pane.
   * Debugger: *.lst and *.lab symbols are automatically detected and loaded for direct-run EXEs and cartridges.
   * Debugger: Improved source-level debugging capability.
   * Debugger: Opcodes are now cached so that they show correctly in the history log even with bank switching.
   * UI: Filter mode is now saved on exit.
   * UI: Added command to copy current frame to the clipboard.
   * UI: Added paddle support.
   * GTIA: Optimized artifacting code.
   * GTIA: Added the ability to disable collisions for cheating.
   * Cartridge: Added support for 64K XEGS and 512K MegaCart cartridge types.
   * Cartridge: Added support for SuperCharger 3D cartridge.
   * Cassette: Added support for non-standard baud rates.
   * Cassette: Added auto-boot option.
   * Cassette: Device open commands are now accelerated.

   ### bugs fixed
   * Simulator: More fixes to executable loader.
   * Simulator: 320K memory mode now banks ANTIC along with the CPU.
   * Simulator: Joystick and bank switch masking via the PIA data direction register is now supported.
   * Simulator: Rewrote BASIC handling to use the OPTION switch rather than masking PORTB bit 1.
   * Simulator: Reset PIA, ANTIC, and CPU on warm reset for XL hardware.
   * Simulator: Self-test ROM no longer appears if kernel ROM is disabled.
   * Simulator: Adjusted PAL frame rate.
   * Simulator: Disk boot flag is now set when loading EXEs to accommodate programs that warm start through DOSINI vector.
   * Simulator: EXE loader is more tolerant of broken files.
   * CPU: Fixed many 65C02/65C816 bugs.
   * CPU: Fixed emulation of NMOS 6502 decimal mode.
   * CPU: Fixed bugs in undocumented instructions.
   * CPU: Fixed emulation crash when switching to 65C816 on the fly.
   * CPU: Fixed handling of break flag.
   * CPU: Now allow one instruction to pass before taking IRQ interrupt after clearing I flag.
   * Debugger: Improved handling of 65C816 modes in disassembler.
   * Debugger: Debug memory reads no longer see $FF in the page where a read memory breakpoint is set.
   * Debugger: Fixed bugs in symbol parser when loading MADS listings.
   * Debugger: Fixed line highlighting in source windows.
   * Debugger: Disassembly recomputes on 65C816 mode changes.
   * Disk: Fixed bug in loading of ATR files that have non-standard boot sector counts.
   * Disk: Additional compatibility fixes to DSKINV acceleration.
   * Disk: Status commands are no longer delayed by the accurate disk timing option.
   * Disk: Applied slight adjustment to SIO transfer rate.
   * Disk: POKEY timers are now shut off after SIO/DSKINV acceleration.
   * Disk: Strengthened disk image format validation and made XFD loading more tolerant.
   * UI: Fixed bug in CPU mode dialog that prevented selection of 65C02 mode.
   * UI: Fixed crash when accessing menu with display pane closed.
   * UI: Keyboard-as-joystick logic no longer allows simultaneous up+down and left+right.
   * UI: Added support for controlling joystick port 2.
   * UI: Blocked invalid machine configurations with XL kernel and 800 hardware or less than 64K of memory.
   * ANTIC: Fixed broken mode 9.
   * ANTIC: Vertical reflect now works on modes 4-7.
   * ANTIC: Player/missile DMA no longer occurs during vertical blank.
   * ANTIC: Fixed behavior of vertical scrolling on blank lines.
   * ANTIC: Changes in playfield width are now supported later in a scan line.
   * ANTIC: Moved VCOUNT advance forward by one cycle.
   * ANTIC: NMIs are now suppressed if enabled too close to trigger point.
   * ANTIC: NMIs are now delayed by one cycle if enabled exactly on cycle 8.
   * GTIA: Bogus missile-player collisions were reported if a register change occurred in the middle of a missile.
   * GTIA: Fixed CONSOL I/O interactions on switch lines.
   * GTIA: Save states now restore player/missile widths.
   * HLE: DSKINV now supports double-density disks.
   * HLE: Improved compatibility of post-init PORTB state.
   * POKEY: Rewrote resampling filter to better block high frequencies.
   * POKEY: Fixed 17-bit noise register polynomial and shift direction.
   * POKEY: Initialization mode (SKCTL[1:0] == 0) now resets the low-speed clocks and polynomial shift registers.
   * POKEY: Asynchronous serial receive mode no longer suspends channels 3 and 4.

## Version 1.2 [July 5, 2009]:
   ### features added
   * Debugger: Added memory pane.
   * Debugger: Shift+F11 is now a shortcut for Step Out.
   * Debugger: History window now supports the mouse wheel.
   * GTIA: Added artifacting support.
   * UI: Cursor auto-hides when over display pane.
   * UI: Added window pane layout saving.
   * UI: Basic support for an Atari ST mouse in port 2.
   * UI: Added more keyboard mappings.
   * Simulator: Added initial save state support.
   * Simulator: Added option to pause when not the active application.
   * CPU: Initial 65C02 and 65C816 support.

   ### bugs fixed
   * Simulator: Fixed RUNAD handling in executable loader.
   * Simulator: Fixed incorrect TRIG3 cartridge sense on launch.
   * UI: Tweaked aspect ratio constants.
   * UI: Fixed some bugs with pane undocking.
   * ANTIC: Added missing address wrapping (fixes hang loading Bounty Bob).
   * ANTIC: Fixed VSCROL timing to acccommodate more mode line extension tricks.
   * ANTIC: Fixed low bits in NMIST register.
   * POKEY: Tweaked hi-pass to sound better.
   * POKEY: Implemented fast pot scan mode.
   * POKEY: Two-tone serial mode now affects timer 1/2 sound output.
   * Debugger: Breakpoints can now be seen on the active line.
   * Debugger: Fixed early stop when trying to Step Out across a PLA or PLP instruction.
   * Debugger: CPU status traces on single-step use the instruction PC rather than the raw PC.
   * HLE: Fixed black status pane in Pole Position due to incorrect attract mode handling.
   * HLE: Fixed startup path problems that prevented SpartaDOS X from starting.
   * HLE: Screen handler now adjusts user memory limit.
   * HLE: Fixed length return value bug in CIO put characters command.
   * HLE: Control codes now work with S:.
   * Disk: Fixed CIO return codes for H: handler.
   * Cartridge: Cart bank is now reset on cold reset.
   * Cartridge: Fixed banking and recognition problems with MaxFlash 1M cartridge format.
   * CPU: Added option to halt on BRK instructions.
   * GTIA: Fixed nasty bug where P/M collisions got messed up whenever the simulator dropped a frame.
   * GTIA: Fixed behavior of sprites when repositioned during active region.
   * GTIA: Implemented text blink and vertical reflect bits.
   * GTIA: Fixed low bit in PAL register in NTSC mode.

## Version 1.1 [May 12, 2009]:
   ### features added
   * CPU: Added an option to disable illegal opcode support.
   * Disk: Added .XFD support.
   * Disk: Added rudimentary hard disk (H:) support.
   * UI: Added exception handler to write crash information.
   * UI: F8 now also breaks into the debugger.
   * UI: Fixed some window activation issues in the docking system.
   * UI: Display is now locked to correct aspect ratio.
   * UI: Dockable panes are now based on percentage rather than absolute width.
   * UI: Changed location of Registry key to work around antivirus false positives.
   * Cartridge: Added support for cartridge images that have headers.
   * Cartridge: Added support for MaxFlash 128K and 1M cartridge formats.
   * Debugger: F5/F8/F9/F10/Shift+F10 now work.
   * Debugger: Frames are now double-clickable in the call stack window.
   * Debugger: F10 now works in the console command line.
   * Debugger: Added history window.
   * HLE: Rewrote HLE kernel mode to be based off the LLE kernel for better compatibility.

   ### bugs fixed
   * Simulator: Init and run vectors are now implemented properly -- fixes New Year's Miracle demo.
   * Simulator: $FFFF optional headers in executables no longer produce load errors.
   * Kernel: DRKMSK was not set -- fixes missing status bar in The Last Word.
   * Debugger: The start of instruction PC is now reported instead of the CPU raw internal PC.
   * Debugger: Fixed infinite loop when viewing disassembly beyond $FFFF.
   * Debugger: Disassembly view edit window now works.
   * Debugger: Fixed a cycle being lost by the CPU on each step or breakpoint hit.
   * Debugger: Console window now scrolls to bottom when updating.
   * UI: Implemented some Ctrl+Key combinations.
   * UI: Fixed jittering problems in full-screen mode.
   * UI: Fixed some focus problems when docking and undocking panes.
   * UI: Full-screen mode no longer shows window borders when using GDI or DirectDraw for display.
   * UI: Removed horizontal and vertical blank regions from display window.
   * UI: Caps Lock now works.
   * UI: Mouse pointer is now hidden in full-screen mode.
   * Disk: Improved disk emulation compatibility with SpartaDOS X.
   * Disk: Fixed corruption of first byte on sector writes.
   * Pokey: Added missing channel 2+4 high pass filter.
   * Pokey: Fixed off-by-one in 1.79MHz 16-bit timing that screwed up some demos using 4-bit polynomial beats.
   * Pokey: Pot scanning is now implemented.
   * GTIA: Color mixing with PRIOR D0-D3=0 now works.
   * GTIA: Fixed incorrect luminance for background in 16 color mode.
   * GTIA: Fixed player/playfield priorities in 9 color mode.
   * GTIA: Fifth player now mixes in PF3 color properly in 16 color/luma modes.
   * GTIA: Color registers no longer have four luma bits.
   * GTIA: ANTIC modes other than 2, 3, and F now work with GTIA modes.
   * GTIA: Implemented psuedo-mode-E caused by resetting the GTIA mode mid-scanline.
   * GTIA: Collisions now register in 9 color mode.
   * GTIA: Implemented one color clock shift in 9 color mode.
   * GTIA: Changing HSCROL in the middle of a mode line now works.
   * ANTIC: Corrected WSYNC, DLI, and VBI timing.
   * CPU: Fixed instruction timing for read-modify-write opcodes using abs,X addressing.

## Version 1.0:
   * Initial release.