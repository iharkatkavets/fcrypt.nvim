# FCrypt.nvim

FCrypt is a Neovim plugin to encrypt and decrypt files

# Demo

![fcrypt](https://github.com/user-attachments/assets/41ff6e3c-3509-4fe4-82e7-7b207d7dcfe6)


## Installation
Using [lazy.nvim](https://github.com/folke/lazy.nvim)
```lua
return {
  'iharkatkavets/fcrypt.nvim',
  build = 'make',
  version = '*',
  cmd = { 'FCryptDecryptBuf', 'FCryptEncryptBuf', 'FCryptVersion', 'FCryptLogs' },
  config = function()
    require('fcrypt').setup()
  end,
}
```
