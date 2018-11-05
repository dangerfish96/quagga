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
nnoremap <F5> :!sudo ./i2rs -f i2rs.conf -A 0.0.0.0 -P 26666<cr>
syntax on
" Uncomment the following to have Vim jump to the last position when                                                       
" " reopening a file
 if has("autocmd")
   au BufReadPost * if line("'\"") > 0 && line("'\"") <= line("$")
    \| exe "normal! g'\"" | endif
endif
