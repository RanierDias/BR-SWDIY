# Linux Packaging

## PT-BR

Arquivos de empacotamento Linux para a GUI `Apus`.

### O que existe aqui

- `apus.desktop` - atalho de menu/aplicativo
- `postinst` - atualiza caches de desktop e icones apos instalar
- `prerm` - hook simples de remocao
- `postrm` - atualiza caches apos remover

### Fluxo de build

1. Gere a build standalone do Nuitka:

```bash
python3 ./build_nuitka_linux.py
```

2. Gere o pacote `.deb`:

```bash
python3 ./build_deb_linux.py
```

Ou use os atalhos shell:

```bash
bash ./build_nuitka_linux.sh
bash ./build_deb_linux.sh
```

### Saidas esperadas

- `dist/linux/apus.dist/` - build standalone do app
- `dist/apus_<version>_<arch>.deb` - pacote Debian

## EN

Linux packaging files for the `Apus` GUI.

### Contents

- `apus.desktop` - application launcher entry
- `postinst` - refreshes desktop/icon caches after install
- `prerm` - simple removal hook
- `postrm` - refreshes caches after uninstall

### Build flow

1. Generate the Nuitka standalone build:

```bash
python3 ./build_nuitka_linux.py
```

2. Generate the `.deb` package:

```bash
python3 ./build_deb_linux.py
```

Or use the shell shortcuts:

```bash
bash ./build_nuitka_linux.sh
bash ./build_deb_linux.sh
```

### Expected outputs

- `dist/linux/apus.dist/` - standalone app build
- `dist/apus_<version>_<arch>.deb` - Debian package
