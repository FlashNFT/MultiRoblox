# MultiRoblox

Aplicativo Windows que permite abrir múltiplas instâncias do Roblox simultaneamente.

## Como funciona

O Roblox usa um Mutex chamado `ROBLOX_singletonEvent` para impedir que mais de uma instância rode ao mesmo tempo. O MultiRoblox cria esse Mutex **antes** do Roblox, "roubando" a posse dele. Assim, quando o Roblox tenta criar o mesmo Mutex, ele já existe mas pertence a outro processo — permitindo que múltiplas instâncias do Roblox abram.

Toda a "mágica" está em uma única linha de código:

```c
CreateMutex(0, 1, L"ROBLOX_singletonEvent");
```

## Como usar

1. Abra o **MultiRoblox** primeiro
2. Depois abra quantas instâncias do Roblox quiser
3. Quando fechar o MultiRoblox, apenas uma instância do Roblox continuará aberta

## Como compilar

### Visual Studio

1. Abra o arquivo `MultiRoblox.sln` no Visual Studio 2022 (ou superior)
2. Selecione a configuração desejada (Debug/Release, x86/x64)
3. Compile com `Ctrl+Shift+B`

### MinGW / GCC

```bash
gcc -o MultiRoblox.exe MultiRoblox/main.c MultiRoblox/gui.c -lgdi32 -luser32 -lkernel32 -mwindows
```

## Notas importantes

- **Apenas Windows**: este app usa a Win32 API e só funciona no Windows
- **Abra antes do Roblox**: o MultiRoblox deve ser iniciado **antes** de abrir qualquer instância do Roblox
- Inspirado no projeto [Dashbloxx/MultiRoblox](https://github.com/Dashbloxx/MultiRoblox)

## Licença

MIT License — veja o arquivo [LICENSE](LICENSE) para detalhes.
