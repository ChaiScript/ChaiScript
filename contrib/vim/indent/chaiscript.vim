" Vim indent file
" Language:	Minnow
" Maintainer:	Jason Turner <jason 'at' emptycrate.com>

" Only load this indent file when no other was loaded.
if exists("b:did_indent")
  finish
endif
let b:did_indent = 1

setlocal indentexpr=GetMinnowIndent()

" To make Vim call GetMinnowIndent() when it finds '\s*end' 
" on the current line ('else' is default and includes 'elseif').
setlocal indentkeys+=0=end

setlocal autoindent

" Only define the function once.
if exists("*GetMinnowIndent")
  finish
endif

function! GetMinnowIndent()
  " Find a non-blank line above the current line.
  let lnum = prevnonblank(v:lnum - 1)

  " Hit the start of the file, use zero indent.
  if lnum == 0
    return 0
  endif

  " Add a 'shiftwidth' after lines that start a block:
  " 'function', 'if', 'for', 'while', 'repeat', 'else', 'elseif', '{'
  let ind = indent(lnum)
  let flag = 0
  let prevline = getline(lnum)
  if prevline =~ '^.*{.*'
    let ind = ind + &shiftwidth
    let flag = 1
  endif

  " Subtract a 'shiftwidth' after lines ending with
  " 'end' when they begin with 'while', 'if', 'for', etc. too.
  if flag == 1 && prevline =~ '.*{.*}.*'
    let ind = ind - &shiftwidth
  endif

  " Subtract a 'shiftwidth' on end, else (and elseif), until and '}'
  " This is the part that requires 'indentkeys'.
  if getline(v:lnum) =~ '^\s*\%(}\)'
    let ind = ind - &shiftwidth
  endif

  return ind
endfunction
