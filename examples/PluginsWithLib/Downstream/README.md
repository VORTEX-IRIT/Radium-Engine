# Plugin sharing library - Downstream

This example illustrates how to use a library shipped from another plugin (should be installed first).

## Configure

`cmake -DRadium_DIR=/pathToInstalledRadium/lib/cmake/Radium/ -DExampleLibraryUpstream_DIR=/pathToInstalledUpstreamLibrary ..`

If Qt6 package is not found, add the option `-DQt6_DIR=/pathToInstalledQt6/lib/cmake/Qt6` to the cmake command.

## Compile

```bash
make
```

The plugin can be loaded from the build tree.

## Install

```bash
make install
```

The plugin can be loaded from the install tree.
