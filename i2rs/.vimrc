set tabstop=4
set softtabstop=4
set shiftwidth=4
set noexpandtab
set colorcolumn=110
highlight ColorColumn ctermbg=darkgray
augroup project
    autocmd!
        autocmd BufRead,BufNewFile *.h,*.c set filetype=c.doxygen
augroup END
let &path.="src/include,/usr/include/AL,"
set includeexpr=substitute(v:fname,'\\.','/','g')
set makeprg=make\ -j6
nnoremap <F4> :make!<cr>
nnoremap <F5> :!./i2rs<cr>
syntax on
