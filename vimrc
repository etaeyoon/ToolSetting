call plug#begin('~/.vim/plugged')

Plug 'scrooloose/nerdtree', {'on': 'NERDTreeToggle'}

Plug 'majutsushi/tagbar' , {'on': 'Tagbar'}

Plug 'wesleyche/SrcExpl', {'on': 'SrcExpl'}

Plug 'altercation/vim-colors-solarized'

Plug 'ronakg/quickr-cscope.vim'

Plug 'vimwiki/vimwiki'

Plug 'ycm-core/youcompleteme', {'do': 'python3.8 ./install.py --clang-completer'}

Plug 'SirVer/ultisnips'

call plug#end()

"after sudo apt install build-essential cmake vim python3-dev, ycm will be
"installed 
"Plug 'MarcWeber/vim-addon-mw-utils'
"Plug 'tomtom/tlib_vim'
"Plug 'garbas/vim-snipmate'

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

" vimwiki set
" https://johngrib.github.io/wiki/vimwiki/#vimwiki-%EC%84%A4%EC%B9%98-%EB%B0%A9%EB%B2%95

" 로컬 리더 키 설정은 취향이니 각자 마음에 드는 키로 설정한다
let maplocalleader = "\\"

"1번 위키(공개용)와 2번 위키(개인용)
let g:vimwiki_list = [
    \{
    \   'path': '/mnt/d/UbuntuSpace/git/wiki/',
    \   'ext' : '.md',
    \   'diary_rel_path': '.',
    \},
\]

" vimwiki의 conceallevel 을 끄는 쪽이 좋다
let g:vimwiki_conceallevel = 0

" 자주 사용하는 vimwiki 명령어에 단축키를 취향대로 매핑해둔다
command! WikiIndex :VimwikiIndex
nmap <LocalLeader>ww <Plug>VimwikiIndex
nmap <LocalLeader>wi <Plug>VimwikiDiaryIndex
nmap <LocalLeader>w<LocalLeader>w <Plug>VimwikiMakeDiaryNote
nmap <LocalLeader>wt :VimwikiTable<CR>

" F4 키를 누르면 커서가 놓인 단어를 위키에서 검색한다.
nnoremap <F4> :execute "VWS /" . expand("<cword>") . "/" <Bar> :lopen<CR>

" Shift F4 키를 누르면 현재 문서를 링크한 모든 문서를 검색한다
nnoremap <S-F4> :execute "VWB" <Bar> :lopen<CR>

" wiki updated
let g:md_modify_disabled = 0

function! LastModified()
  if g:md_modify_disabled
    return
  endif
  if &modified
    let save_cursor = getpos(".")
    let n = min([10, line("$")])
    keepjumps exe '1, ' . n . 's#^\(.\{,10}updated\s*: \).*#\1 .
          \ strftime('%Y-%m-%d %H:%M:%S +0900') . '#e'
    call histdel('search', -1)
    call setpos('.', save_cursor)
  endif
endfunction

"wiki template
function! NewTemplate()
  let l:wiki_directory = v:false

  for wiki in g:vimwiki_list
    if expand('%:p:h') == wiki.path
      let l:wiki_directory = v:true
      break
    endif
  endfor

  if !l:wiki_directory
    return
  endif

  if line("$") > 1
    return
  endif

  let l:template = []
  call add(l:template, '---')
  call add(l:template, 'layout  : wiki')
  call add(l:template, 'title   : ')
  call add(l:template, 'summary : ')
  call add(l:template, 'date    : ' . strftime('%Y-%m-%d %H:%M:%S +0900'))
  call add(l:template, 'updated : ' . strftime('%Y-%m-%d %H:%M:%S +0900'))
  call add(l:template, 'tags    : ')
  call add(l:template, 'toc     : true')
  call add(l:template, 'public  : true')
  call add(l:template, 'parent  : ')
  call add(l:template, 'latex   : false')
  call add(l:template, '---')
  call add(l:template, '* TOC')
  call add(l:template, '{:toc}')
  call add(l:template, '')
  call add(l:template, '#')
  call setline(1, l:template)
  execute 'normal! G'
  execute 'normal! $'

  echom 'new wiki page has created'
endfunction

augroup vimwikiauto
  autocmd BufWritePre *wiki/*.md call LastModified()
  autocmd BufRead,BufNewFile *wiki/*.md call NewTemplate()
augroup END

