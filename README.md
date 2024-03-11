# Simple application wrapper

```
> kshimgen
KShimgen 0.6.0:
--create shim target                    Create a shim
--env key=val                           additional environment varriables for the shim
--enable-env-override                   whether to allow overriding the target with the env var KSHIM_shim
--gui                                   create a gui application (only supported on Windows)
-- arg1 arg2 arg3...                    arguments that get passed to the target
```

## Architecture
The project consists out of two main components:

### `kshim`
 A simple binary containing a JSON string describing the target application to run and some options that are passed to the target.

### `kshimgen`
`kshimgen` has an embedded copy of `kshim`.
 When invoked with `--create`, the embedded version of `kshim`is written to the target location and the contained JSON string is binary patched with the options.

