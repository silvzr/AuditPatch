Check https://android-review.googlesource.com/c/platform/system/logging/+/3725346 for details.



Thanks to [brunoanc's fork](https://github.com/brunoanc/AuditPatch) for the inspiration and partial code base.



Uses [linjector-rs](https://github.com/erfur/linjector-rs) in order to inject our lib on post-fs-data stage instead of relying on ZygiskNext API while not having the need to restart logd itself, and [PerformanC's LSPlt](https://github.com/PerformanC/LSPlt) for PLT hooks.

