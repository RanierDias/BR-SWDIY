# Linux Packaging

This folder contains Linux packaging helpers for the BR-SWDIY GUI.

Files:

- `apus.desktop`: desktop launcher template for Ubuntu/Debian desktop environments
- `postinst`: refreshes desktop and icon caches after install
- `prerm`: package removal hook placeholder
- `postrm`: refreshes desktop and icon caches after removal

Build outputs:

- standalone app staged in `dist/linux/`
- Debian package in `dist/apus_<version>_<arch>.deb`

Suggested install command:

```bash
sudo dpkg -i ./dist/apus_<version>_<arch>.deb
```

If dependencies are missing:

```bash
sudo apt -f install
```
