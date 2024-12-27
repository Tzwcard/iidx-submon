# iidx-submon
A sub monitor that mimic how old and new beatmaniaIIDX front panel can do.

## Example
```C++
...
typedef int(*callable_iidx_submon_init)(void);
typedef bool(*callable_iidx_submon_set_16seg)(unsigned char *);
typedef void(*callable_iidx_submon_finl)(void);

static HMODULE hMod;
static callable_iidx_submon_init iidx_submon_init;
static callable_iidx_submon_set_16seg iidx_submon_set_16seg;
static callable_iidx_submon_finl iidx_submon_finl;
...
    hMod = LoadLibrary(L"iidx-submon.dll");
    if (hMod) {
        iidx_submon_init = (hMod)GetProcAddress(hmodule_submon, "iidx_submon_init");
        iidx_submon_get_resist = (callable_iidx_submon_get_resist)GetProcAddress(hMod, "iidx_submon_get_resist");
        iidx_submon_set_16seg = (callable_iidx_submon_set_16seg)GetProcAddress(hMod, "iidx_submon_set_16seg");
        iidx_submon_finl = (callable_iidx_submon_finl)GetProcAddress(hMod, "iidx_submon_finl");

        if (iidx_submon_init
            && iidx_submon_finl) {
            log_misc("%-24s: [%p]", "hmodule_submon", hMod);
            log_misc("%-24s: [%p]", "iidx_submon_init", iidx_submon_init);
            log_misc("%-24s: [%p]", "iidx_submon_set_16seg", iidx_submon_set_16seg);
            log_misc("%-24s: [%p]", "iidx_submon_finl", iidx_submon_finl);

            running_submon = iidx_submon_init() > 0 ? true : false;
        }
    }
...
    if (running_submon && iidx_submon_set_16seg) {
        iidx_submon_set_16seg(pout_staging.SEG16);
    }
...
    if (running_submon && iidx_submon_finl) {
        iidx_submon_finl();
    }
...
```

Functions are defined in ``iidx-submon.cpp`` and ``exports.def``, please check the comments in cpp file.

## Screenshots
![](/imgs/effector.svg)
![](/imgs/ttresist.svg)
![](/imgs/ledcontrol.svg)
![](/imgs/keypad.svg)
