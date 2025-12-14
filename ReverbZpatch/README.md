# ReverbZpatch

ReverbZ implementation for the Daisy Patch SM (in this case, with patch.Init() hardware).

To use this project as a template, use the following helper.py command: (not using python3 can result in 'permission denied')

```bash
python3 ./helper.py copy MyProjects/MyAwesomeNewProject --source patch_sm/ReverbZpatch
```

To compile from anywhere: 
```bash
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/OwnProjects/ReverbZpatch
```

To force a clean build of the project:
```bash
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/OwnProjects/ReverbZpatch clean all
```

To program with STLink:
```bash
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/OwnProjects/ReverbZpatch program
```

[To do both]:
```bash
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/OwnProjects/ReverbZpatch clean all program
```

DEBUG BUILD (No Optimization):
```bash
# libDaisy
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/libDaisy clean
OPT='-O0 -g3' make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/libDaisy

# DaisySP
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/DaisySP clean
OPT='-O0 -g3' make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/DaisySP

# dspLib -- > skip build, it is not configured and built by ReverbZpatch
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/dspLib clean
OPT='-O0 -g3' make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/dspLib

# ReverbZpatch
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/OwnProjects/ReverbZpatch clean
BUILD_TYPE=debug make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/OwnProjects/ReverbZpatch all
```

Release Build (Optimized - for production)
```bash
# libDaisy
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/libDaisy clean
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/libDaisy

# DaisySP
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/DaisySP clean
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/DaisySP

# dspLib
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/dspLib clean
BUILD_TYPE=release make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/dspLib

# ReverbZpatch
make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/OwnProjects/ReverbZpatch clean
BUILD_TYPE=release make -C /Users/matteodesantis/EurorackProgramming/Git/DaisyDev/OwnProjects/ReverbZpatch all
```

## Controls

None
