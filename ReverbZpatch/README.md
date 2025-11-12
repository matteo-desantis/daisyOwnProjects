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

## Controls

None
