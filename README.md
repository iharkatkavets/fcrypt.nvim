# FCrypt.nvim

FCrypt is a Neovim plugin to encrypt and decrypt files

## Installation
Using [lazy.nvim](https://github.com/folke/lazy.nvim)
```lua
return {
  'iharkatkavets/fcrypt.nvim',
  build = 'make',
  version = '*',
  cmd = { 'FCryptDecryptBuf', 'FCryptEncryptBuf', 'FCryptVersion', 'FCryptLogs' },
}
```
