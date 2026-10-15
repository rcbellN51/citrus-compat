# citrus-compat

Experimental: Developed and tested on MINIX 3.4.0. Compatibility with other MINIX releases has not been established.

A project-local compatibility layer that restores dynamic Citrus internationalization support on MINIX 3.

The library enables applications to use MINIX's existing Citrus encoding modules for multibyte character conversion, locale handling, and `iconv()` without rebuilding libc or modifying system libraries.

## Background

MINIX 3 includes components of the Citrus internationalization framework, including encoding modules, locale data, and support for character-set conversion.

However, the MINIX 3.4.0 libc examined during development was built without dynamic Citrus module support (`_I18N_DYNAMIC`).

Several libc functions required for dynamic internationalization are consequently implemented as stubs. These prevent the existing encoding modules from being loaded and initialized through the normal Citrus interfaces.

For example, before using the compatibility layer:

- `setlocale(LC_CTYPE, "en_US.UTF-8")` returned `NULL`.
- The UTF-8 CTYPE module could not be initialized.
- The standard encoding interface could not dynamically load UTF-8 support.

The underlying encoding modules were already present. The missing functionality was primarily the connection between libc and those modules.

## Approach

Rather than rebuilding or replacing libc, `citrus-compat` supplies implementations of the missing dynamic Citrus entry points.

These symbols are linked directly into the application, allowing them to take precedence over the corresponding implementations in MINIX libc.

The compatibility layer delegates character conversion to MINIX's original Citrus modules.

It does not implement its own UTF-8 decoder or encoder.

No system libraries, locale databases, or encoding modules are replaced.

## Components

### citrus-loader.c

Implements dynamic Citrus module management:

- `_citrus_load_module`
- `_citrus_find_getops`
- `_citrus_unload_module`

Modules are loaded from `/usr/lib/i18n/` using `dlopen()`.

The required Citrus operations are resolved with `dlsym()`, and modules are released using `dlclose()`.

### citrus-stdenc.c

Implements the dynamic standard encoding interface:

- `_citrus_stdenc_open`
- `_citrus_stdenc_close`

The implementation loads the appropriate encoding module, retrieves its STDENC operations table, initializes the encoding object, and performs cleanup when the object is closed.

The original libc implementation is retained for the built-in `NONE` encoding.

The recovered STDENC interface uses ABI version 2.

### citrus-ctype.c

Implements the dynamic character-type interface:

- `_citrus_ctype_open`
- `_citrus_ctype_close`

This interface supports locale-dependent multibyte conversion through the existing Citrus CTYPE modules.

The original libc implementation is retained for the built-in `NONE` encoding.

The recovered CTYPE interface uses ABI version 3.

The compatibility implementation also preserves the initialization and cleanup behavior required by the original modules, including handling failed initialization without calling uninitialization routines on objects that were never
successfully initialized.

## Building

The project uses the MINIX C compiler and BSD make.

From the project directory:

```sh
make
```

This builds:

```text
libcitrus-compat.a
```

To build the example application:

```sh
make examples
```

To build and run the automated tests:

```sh
make check
```

For a complete rebuild and verification:

```sh
make clean
make
make examples
make check
```

The compatibility library is built locally. No installation into system library directories is required.

## Linking applications

MINIX libc already defines the Citrus entry points that `citrus-compat` replaces.

Consequently, ordinary static archive linking may not extract the compatibility objects from `libcitrus-compat.a`, because the linker considers the symbols satisfied by libc.

Applications should link the complete compatibility archive using:

```sh
-Wl,--whole-archive ./libcitrus-compat.a -Wl,--no-whole-archive
```

For example:

```sh
cc -o myprogram myprogram.c \
    -Wl,--whole-archive ./libcitrus-compat.a -Wl,--no-whole-archive
```

The `--whole-archive` option ensures that all compatibility objects are included in the executable, making their implementations available in preference to the libc stubs.

This requirement is specific to linking against an archive containing replacement symbols that libc already provides.

## Tests

The project includes tests for module loading, encoding initialization, locale handling, conversion, and error cleanup.

The automated test suite includes:

| Test | Purpose |
|---|---|
| `integration-test` | Exercises locale initialization, multibyte conversion, `iconv()`, and locale switching |
| `integration-whole` | Verifies integration using whole-archive linking |
| `error-test` | Tests invalid encoding handling and subsequent successful conversion |
| `stdenc-error-test` | Tests missing modules and unavailable STDENC operations |
| `stdenc-fault-test` | Tests STDENC ABI validation, initialization failures, and cleanup |
| `ctype-fault-test` | Tests CTYPE ABI validation, initialization failures, and cleanup |

Additional development tests exercise individual Citrus interfaces and locale behavior.

### UTF-8 example

The example program `examples/utf8-demo.c` demonstrates decoding and round-trip encoding of:

| Character | Unicode value | UTF-8 bytes |
|---|---|---|
| Latin small letter e with acute | U+00E9 | `C3 A9` |
| Euro sign | U+20AC | `E2 82 AC` |
| Grinning face | U+1F600 | `F0 9F 98 80` |

These conversions have been verified using MINIX's original UTF-8 module.

### Diagnostic tests

Two additional programs investigate the original module's UTF-8 validation behavior:

- `tests/utf8-invalid-test.c`
- `tests/utf8-boundary-test.c`

These are diagnostic programs rather than automated regression tests. Some cases deliberately check modern Unicode requirements that the original MINIX module does not satisfy.

They are not included in `make check`.

## Known limitations

### Historical UTF-8 behavior

MINIX 3's original `/usr/lib/i18n/libUTF8.so` implements a historical extended UTF-8 encoding scheme that supports sequences of up to six bytes, rather than the modern Unicode UTF-8 limit of four bytes.

Testing and examination of the original module's disassembly established that:

- The decoder accepts five- and six-byte sequences and decoded values above U+10FFFF.
- Overlong encodings, invalid continuation bytes, and UTF-16 surrogate code points are rejected.
- Incomplete multibyte sequences are handled through the conversion state.
- The CTYPE and STDENC interfaces share the same internal UTF-8 decoder.
- The STDENC implementation also contains encoding logic for five- and six-byte sequences.

For example, the module accepts the following sequences, which are invalid under modern Unicode UTF-8 rules:

| Byte sequence | Decoded value |
|---|---|
| `F4 90 80 80` | U+110000 |
| `F8 88 80 80 80` | U+200000 |
| `FC 84 80 80 80 80` | U+4000000 |

This behavior originates in MINIX's existing `libUTF8.so`, not in `citrus-compat`. The compatibility layer restores access to the original Citrus functionality without modifying the encoding algorithms or imposing additional Unicode
validation.

Consequently, applications requiring strict Unicode UTF-8 conformance must perform their own validation or use an implementation that enforces the modern Unicode scalar-value range.

The diagnostic programs `tests/utf8-invalid-test.c` and `tests/utf8-boundary-test.c` demonstrate this behavior. Their modern Unicode conformance failures are expected with the original MINIX module and do not indicate
compatibility-layer regressions.

### Locale object caching

MINIX libc retains initialized LC_CTYPE locale objects for the lifetime of the process.

Switching between locales does not necessarily close or unload previously initialized CTYPE modules.

This behavior belongs to libc's locale management and is not changed by `citrus-compat`.

### Platform and ABI compatibility

The compatibility layer was developed and tested against MINIX 3.4.0, using its existing Citrus modules and internal interfaces.

The STDENC and CTYPE interfaces were reconstructed through examination of the available MINIX binaries and verified through testing.

These are internal interfaces, not a portable public API. Compatibility with other MINIX releases or different Citrus module implementations has not been established.

## Project scope

The goal of `citrus-compat` is to restore access to functionality already present in MINIX.

The project deliberately avoids:

- Rebuilding or replacing MINIX libc.
- Replacing the installed Citrus encoding modules.
- Modifying the system locale or encoding databases.
- Introducing alternative character-encoding algorithms.
- Changing MINIX's existing locale caching behavior.

The library is intended for applications that can be rebuilt and linked with the compatibility archive. It does not automatically modify the behavior of existing executables.

## Related issue

The underlying missing dynamic Citrus functionality is documented in the MINIX issue tracker:

[Stichting-MINIX-Research-Foundation/minix#217](https://github.com/Stichting-MINIX-Research-Foundation/minix/issues/217)

## Development status

The compatibility layer successfully supports the tested UTF-8 locale operations, multibyte character conversion, and `iconv()` integration.

Automated integration and fault-injection tests pass.

The implementation remains experimental and is intended primarily for MINIX 3 development, compatibility testing, and exploration of its existing internationalization infrastructure.

## License

This project is distributed under the MIT License. See [LICENSE](LICENSE) for details.

Development history and provenance are documented in [DEVELOPMENT.md](DEVELOPMENT.md).
