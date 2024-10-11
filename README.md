# Radium-Engine libraries

![banner](doc/images/radium-banner.webp)

Radium is a research 3D Engine for rendering, animation and processing.
It is developed and maintained by the [STORM research group](https://www.irit.fr/STORM/site/).

Radium-Engine comes in two flavors:

- the stable [https://github.com/STORM-IRIT/Radium-Engine/tree/master](master) branch.
- the unstable bleeding edge [https://github.com/STORM-IRIT/Radium-Engine/tree/release-candidate](release-candidate) branch.

<p align="center">
  <b>
  [<a href="https://github.com/STORM-IRIT/Radium-Releases/releases">Click here to download Precompiled binaries</a>]
  </b><br/>
   <b>
  [<a href="https://storm-irit.github.io/Radium-Engine/">Click here for API documentation</a>]
  </b>
</p>

The Radium ecosystem is composed of

- **Radium Libraries**: this project (see documentation here: https://storm-irit.github.io/Radium-Engine/).
  Contains the Radium libraries.
- **Radium Apps** (https://github.com/STORM-IRIT/Radium-Apps): contains several applications, including MainApp,
  which is for now the default software with GUI. This repository also demonstrates command-line applications.
- **Radium Releases** (https://github.com/STORM-IRIT/Radium-Releases): meta-repository generating precompiled binaries
  for Radium libraries and its applications.
- **Radium Plugins Example** (https://github.com/STORM-IRIT/Radium-PluginExample): contains plugins examples,
  demonstrating how to write, compile and use plugins with MainApp.
- **Radium Official Plugins** (https://gitlab.com/Storm-IRIT/radium-official-plugins): general purpose plugins.

## Which repository should I use ?

Depending on you needs, you may want to:

- Use Radium as a viewer and processing software, with graphical user interface.
  That's the most simple case: simply download the latest release, and run.
  See https://github.com/STORM-IRIT/Radium-Releases/releases.
- Contribute to Radium, or develop your own application of plugin:
  **Radium Libraries** is the default and mandatory repository you need to fetch first.
  If you only want to _use_ Radium, compile and install it (see [documentation](https://storm-irit.github.io/Radium-Engine/)).
  Then,
      - If you want to contribute to the application or plugins developments, fetch and compile the **Radium Apps** and
        **Radium Official Plugins** respectively.
      - If you want to write your own application of plugin, checkout the
        [HelloRadium](https://github.com/STORM-IRIT/Radium-Apps/tree/fix-compilation/HelloRadium) and
        **Radium Plugins Example** projects.

## Reference

If you use Radium-Engine for your research and publication, or simply want to refer/cite Radium Engine, please use the following

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.5101334.svg)](https://doi.org/10.5281/zenodo.5101334)

```{}
@software{RadiumEngine,
  author       = {Charly Mourglia and Valentin Roussellet and
                  Loïc Barthe and Nicolas Mellado and
                  Mathias Paulin and David Vanderhaeghe and others},
  title        = {Radium-Engine},
  month        = jul,
  year         = 2021,
  note         = {https://storm-irit.github.io/Radium-Engine/},
  publisher    = {Zenodo},
  version      = {doi-1.0.0},
  doi          = {10.5281/zenodo.5101334},
  url          = {https://doi.org/10.5281/zenodo.5101334}
}
```

## Badges

[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/374bd173d03946a9b54c3c6bbd8cd589)](https://app.codacy.com/gh/STORM-IRIT/Radium-Engine?utm_source=github.com&utm_medium=referral&utm_content=STORM-IRIT/Radium-Engine&utm_campaign=Badge_Grade_Settings)
[![codecov](https://codecov.io/gh/STORM-IRIT/Radium-Engine/branch/master/graph/badge.svg?token=MKfANkC3sd)](https://codecov.io/gh/STORM-IRIT/Radium-Engine)

[![CI  windows release float  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/windows-latest.Release.float.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
[![CI  windows debug float  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/windows-latest.Debug.float.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
[![CI  windows release double  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/windows-latest.Release.double.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
[![CI  windows debug double  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/windows-latest.Debug.double.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)

[![CI  macos release float  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/macos-latest.Release.float.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
[![CI  macos debug float  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/macos-latest.Debug.float.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
[![CI  macos release double  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/macos-latest.Release.double.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
[![CI  macos debug double  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/macos-latest.Debug.double.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)

[![CI  linux release float  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/ubuntu-latest.Release.float.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
[![CI  linux debug float  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/ubuntu-latest.Debug.float.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
[![CI  linux release double  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/ubuntu-latest.Release.double.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
[![CI  linux debug double  badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/nmellado/0e76e93f56eba8a7b28d6a0116873d85/raw/ubuntu-latest.Debug.double.json)](https://github.com/STORM-IRIT/Radium-Engine/actions?query=branch%3Amaster+workflow%3A%22CI%22)
