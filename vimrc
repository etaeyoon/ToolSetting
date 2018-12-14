call plug#begin('~/.vim/plugged')

Plug 'scrooloose/nerdtree', {'on': 'NERDTreeToggle'}

Plug 'majutsushi/tagbar' , {'on': 'Tagbar'}

Plug 'wesleyche/SrcExpl', {'on': 'SrcExpl'}

Plug 'MarcWeber/vim-addon-mw-utils'
Plug 'tomtom/tlib_vim'
Plug 'garbas/vim-snipmate'

Plug 'altercation/vim-colors-solarized'

Plug 'ronakg/quickr-cscope.vim'

call plug#end()

function! LoadCscope()
  let db = findfile("cscope.out", ".;")
  if(!empty(db))
    let path = strpart(db, 0, match(db, "/cscope.out$"))
    set nocscopeverbose " suppress 'duplicate connection' error
    exe "cs add " . db . " " . path
    set cscopeverbose
  endif
endfunction
au BufEnter /* call LoadCscope()

set tabstop=2
set expandtab
set shiftwidth=2
syntax enable
set background=dark
