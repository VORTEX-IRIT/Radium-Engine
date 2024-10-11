\page basicsCompileVs Microsoft Visual Studio Compilation Instructions

# Build external dependencies

Open the external folder in VS using "Open a local folder" and edit cmake settings or `CMakeSettings.json`. External build and install have to be outside Radium-Engine source directory. For instance

~~~{.json}
{
  "configurations": [
    {
      "name": "x64-Debug",
      "generator": "Ninja",
      "configurationType": "Debug",
      "inheritEnvironments": [ "msvc_x64_x64" ],
      "buildRoot": "${projectDir}/../../radium-externals/build/${name}",
      "installRoot": "${projectDir}/../../radium-externals/install/${name}",
      "cmakeCommandArgs": "-DCMAKE_PREFIX_PATH=C:/Qt/6.3.0/msvc2019_64",
      "buildCommandArgs": "",
      "ctestCommandArgs": ""
    },
    {
      "name": "x64-Release",
      "generator": "Ninja",
      "configurationType": "Release",
      "buildRoot": "${projectDir}/../../radium-externals/build/${name}",
      "installRoot": "${projectDir}/../../radium-externals/install/${name}",
      "cmakeCommandArgs": "-DCMAKE_PREFIX_PATH=C:/Qt/6.3.0/msvc2019_64",
      "buildCommandArgs": "",
      "ctestCommandArgs": "",
      "inheritEnvironments": [ "msvc_x64_x64" ],
      "variables": []
    }
  ]
}
~~~

Build the project, this will install the external dependencies.

# Build Radium

Open the Radium folder (via "File > Open > Folder..." or `devenv.exe <foldername>`).
VS will run cmake, generate the target builds (Debug and Release by default).
Other build types can be added by editing `CMakeSettings.json`.

Configure cmake option (see official doc [here](https://docs.microsoft.com/cpp/build/customize-cmake-settings))
You have to provide path to Qt installation, glfw installation (for headless support) and external dependencies configuration.
In order to execute Radium demos from the buildtree (installing them takes time due to qt deployement procedure),
you also need to define environment variables that point to the various dlls used by Radium components that will be used on the per-target `launch.vs.json` configuration file.
Note that it is recommended to compile or install glfw as a static library, the example below assume that.

For instance, with directory structure for externals as defined above the configuration is

~~~{.json}
{
    "environments": [
    {
        "QtDir": "C:/Qt/6.3.0/msvc2019_64/",
        "glfwDir" : "C:/path/to/glfwInstallation",
        "ExternalInstallDir": "${projectDir}/../radium-externals/install/${name}/bin;${projectDir}/../radium-externals/install/${name}/cpplocate;${projectDir}/../radium-externals/install/${name}/globjects;${projectDir}/../radium-externals/install/${name}/glbinding",
    }
    ],
    "configurations": [
    {
        "name": "x64-Debug",
        "generator": "Ninja",
        "configurationType": "Debug",
        "inheritEnvironments": [ "msvc_x64_x64" ],
        "buildRoot": "${projectDir}/out/build/${name}",
        "installRoot": "${projectDir}/out/install/${name}",
        "cmakeCommandArgs": "-DCMAKE_PREFIX_PATH=\"${env.QtDir};${env.glfwDir}/lib/cmake\" -C ${projectDir}/../radium-externals/install/${name}/radium-options.cmake",
        "buildCommandArgs": "",
        "ctestCommandArgs": "",
        "environments": [
        {
            "environment": "RadiumDllsLocations",
            "ExternalDllsDIR": "${env.ExternalInstallDir}/${name}/bin;${env.ExternalInstallDir}/${name}/cpplocate;${env.ExternalInstallDir}/${name}/glbinding;${env.ExternalInstallDir}/${name}/globjects",
            "QtDllsDIR": "${env.QtDir}/bin",
            "RadiumDlls": "${buildRoot}/src/Core;${buildRoot}/src/Engine;${buildRoot}/src/Gui;${buildRoot}/src/Headless;${buildRoot}/src/IO;${buildRoot}/src/PluginBase"
        }
        ]
    },
    {
        "name": "x64-Release",
        "generator": "Ninja",
        "configurationType": "Release",
        "inheritEnvironments": [ "msvc_x64_x64" ],
        "buildRoot": "${projectDir}/out/build/${name}",
        "installRoot": "${projectDir}/out/install/${name}",
        "cmakeCommandArgs": "-DCMAKE_PREFIX_PATH=\"${env.QtDir};${env.glfwDir}/lib/cmake\" -C ${projectDir}/../radium-externals/install/${name}/radium-options.cmake",
        "buildCommandArgs": "",
        "ctestCommandArgs": "",
        "environments": [
        {
            "environment": "RadiumDllsLocations",
            "ExternalDllsDIR": "${env.ExternalInstallDir}/${name}/bin;${env.ExternalInstallDir}/${name}/cpplocate;${env.ExternalInstallDir}/${name}/glbinding;${env.ExternalInstallDir}/${name}/globjects",
            "QtDllsDIR": "${env.QtDir}/bin",
            "RadiumDlls": "${buildRoot}/src/Core;${buildRoot}/src/Engine;${buildRoot}/src/Gui;${buildRoot}/src/Headless;${buildRoot}/src/IO;${buildRoot}/src/PluginBase"
        }
        ]
    }
    ]
}
~~~

\note It is strongly encouraged to use `/` separators in your path, instead of `\\`. See <https://stackoverflow.com/questions/13737370/cmake-error-invalid-escape-sequence-u>

\note When compiling the dependencies you may hit the max path length fixed by Microsoft OS (260 characters). To fix this, you might need to change the path of your build dir to shorten it, or to change the limit on your system, see: <https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#enable-long-paths-in-windows-10-version-1607-and-later>

## Compilation

Right click on `CMakeList.txt > build > all`.

## Execution of a demo app

To execute a demo application from the buildtree, i.e. without installing Radium, select in the `Startup Item` list the target you want to execute.
For instance, `DrawPrimitives.exe (examples/...)`.
Then, select in the menu `<Debug>/Debug and Launch Settings for DrawPrimmitives` and modify the `launch.vs.json` file that is opened so that it contains the following.

~~~{.json}
{
 "version": "0.2.1",
 "defaults": {},
 "configurations": [
   {
     "type": "default",
     "project": "CMakeLists.txt",
     "projectTarget": "DrawPrimitives.exe (examples\\DrawPrimitives\\DrawPrimitives.exe)",
     "name": "DrawPrimitives.exe (examples\\DrawPrimitives\\DrawPrimitives.exe)",
     "inheritEnvironments": [ "RadiumDllsLocations" ],
     "env": {
       "PATH": "${env.QtDllsDIR};${env.ExternalDllsDIR};${env.RadiumDlls};${env.PATH}"
     }
   }
 ]
}
~~~

For any target you want to execute, the same should be done, i.e. adding the following to the target configuration

~~~{.json}
 "inheritEnvironments": [ "RadiumDllsLocations" ],
 "env": {
   "PATH": "${env.QtDllsDIR};${env.ExternalDllsDIR};${env.RadiumDlls};${env.PATH}"
 }
~~~

If you plan to execute the 'unittest.exe' target, you should also add the following configuration for the working directory

~~~{.json}
 "currentDir": "${projectDir}/tests/unittest"
~~~

## installation

To install, you need to run any installation target, e.g. `Engine.dll (install)` or to select the menu `<Build>/<Install radiumproject>`
