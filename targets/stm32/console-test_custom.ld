/* Update the linker script with rules for custom sections */
SECTIONS
{
  .text :
  {
    . = ALIGN(4);
    __start_command_section = .;
    *(.command_section)
    __stop_command_section = .;
  } >FLASH
}
