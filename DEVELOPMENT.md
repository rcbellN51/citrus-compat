# Development

`citrus-compat` is an experimental compatibility layer developed for MINIX 3.4.0 to restore dynamic Citrus internationalization functionality without rebuilding or replacing the system C library.

## Development process

The project was developed through iterative AI-assisted collaboration with ChatGPT. Much of the compatibility implementation, test code, and documentation was generated or refined during this collaboration, with the resulting code 
compiled, tested, debugged, and validated on a MINIX 3 system.

Development and testing were performed on MINIX 3.4.0 running in a QEMU/KVM virtual machine.

## Technical investigation

The implementation was developed by examining the behavior and binary interfaces of the existing MINIX libc and Citrus internationalization modules.

Investigation included:

- Identifying Citrus entry points implemented as stubs in MINIX libc.
- Examining exported symbols and operations tables in existing Citrus modules.
- Using binary inspection and disassembly to reconstruct the required CTYPE and STDENC interfaces.
- Determining structure layouts, callback signatures, and ABI versions.
- Testing module initialization, character conversion, error handling, and resource cleanup.
- Investigating the historical UTF-8 behavior of MINIX's original `libUTF8.so`.

The compatibility implementation was written to interact with the existing MINIX interfaces. It does not contain a replacement UTF-8 encoding or decoding algorithm.

The original MINIX source code was not used as the source for the compatibility implementation. The internal interfaces were reconstructed from binary inspection, observed behavior, and iterative testing.

## Implementation scope

The project provides three compatibility components:

- `citrus-loader.c` — dynamic Citrus module loading and symbol resolution.
- `citrus-stdenc.c` — standard encoding interface initialization and cleanup.
- `citrus-ctype.c` — character-type interface initialization and cleanup.

The implementation is linked into applications as a static compatibility archive.

No changes to the installed MINIX libc, Citrus encoding modules, or locale databases are required.

## Testing

The implementation was validated using integration tests, fault-injection tests, locale switching, multibyte character conversion, and `iconv()`.

Additional diagnostic tests were developed to investigate malformed and out-of-range UTF-8 sequences.

These tests established that MINIX's existing UTF-8 module supports historical extended UTF-8 sequences of up to six bytes. This behavior originates in the original MINIX module and is documented separately in `README.md`.

## Limitations

The project was developed and tested against MINIX 3.4.0.

Compatibility with other MINIX releases, other Citrus implementations, or different operating systems has not been established.

The reconstructed interfaces are internal implementation details and may differ between systems or releases.

The project remains experimental.
