# ChronoWall

Gerenciador automático de papel de parede que muda de acordo com períodos do dia.

## Compilando e Instalando

### Método Manual
```bash
# Instalar dependências
sudo apt install build-essential cmake qt6-base-dev   # Debian/Ubuntu
sudo dnf install cmake qt6-qtbase-devel              # Fedora/RHEL
sudo pacman -S cmake qt6-base                        # Arch Linux

# Clonar e compilar
git clone https://github.com/m6rc0sp/chronowall.git
cd chronowall
mkdir build && cd build
cmake ..
make -j$(nproc)

# Instalar
sudo make install
```

## Configuração Inicial

1. Instalar o serviço (primeira vez):
```bash
chronowall --install
```

2. Verificar status do serviço:
```bash
systemctl --user status chronowall
```

3. Ver logs em tempo real:
```bash
journalctl --user -f -u chronowall
```

## Uso

O ChronoWall pode ser executado de três formas:

1. Interface Gráfica:
```bash
chronowall
```

2. Modo Daemon:
```bash
chronowall --daemon
```

3. Através do menu de aplicativos (ChronoWall)

O aplicativo continuará rodando em segundo plano na bandeja do sistema.

## Estrutura do Projeto

```
chronowall/
├── src/
│   ├── core/           # Lógica principal
│   ├── ui/            # Interface do usuário
│   ├── utils/         # Utilitários
│   └── main.cpp
├── resources/         # Recursos do sistema
└── CMakeLists.txt
```

## Empacotamento

### Debian/Ubuntu (deb)
```bash
cd packaging/debian
dpkg-buildpackage -b -us -uc
```

### Fedora/RHEL (rpm)
```bash
cd packaging/rpm
rpmbuild -ba chronowall.spec
```

### Arch Linux
```bash
cd packaging/arch
makepkg -si
```

## Desinstalação

1. Parar e desabilitar o serviço:
```bash
systemctl --user stop chronowall
systemctl --user disable chronowall
```

2. Remover arquivos:
```bash
sudo make uninstall    # Se instalado via make
```

3. Limpar configurações (opcional):
```bash
rm -rf ~/.config/chronowall
rm -rf ~/.cache/chronowall
```

## Contribuindo

Sinta-se à vontade para abrir issues ou enviar pull requests!
