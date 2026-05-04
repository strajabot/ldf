LDF - Lib Defer Print

  LDF is going to be a pluggable header-based library for use in semi-hosted embedded environemnts where executing string formatting code is wasteful in:
  1. codesize
  2. performance

  It works by encoding the format string into a metadata section for later use by the host. the format on device will push into the buffer pointers into the section
  containing the metadata that is not loaded onto the device, and the parameters of the format into the buffer. the buffer is then sent to the host over uart or simply
  stored in memory, and host reads it and using the metadata from the elf, it decodes the formated strings and prints them. this allows one to easily implement debug
  printing.

  I need to have the following steps:

  1. format api takesa sstring containing the format , and args...
  2. LayoutOptimizer shuffles args such that they are ordered by alignment, but keeps track of the mapping original indicies to shuffled indicies. this should work as a
  constexpr mapping
  3. something like the "FMTStringEncoder" goes and does the following things:
     4. parses over the sstring in constexpr evaluation
     5. for each {} it injects the index in the shuffled array from the current index or remaps the provided index into the shuffled index
     6. injects :[typeinfo]: for the related type of arg parsing. this is used to infer the type on the host side
     7. afterwards it typechecks the aditional format specifier info and ads it after the typeinfo

  - this means that after this pass ".. {1:x} .." should become ".. {2:u32:x} .." if the type of arg was uint32_t and it was reordered to the index 2 because there is a
  third arg that is uint64_t (or any with larger alignment)

  the FMTStringEncoder returns a sstring after the resulting transformation...

  4. after this the next step in the process is to use LDF_CREATE_META to store the SSTRING in metadata section.
  5. after that the pointer to the metdata is stored in the buffer, and after that al the args while respecting alignment
  