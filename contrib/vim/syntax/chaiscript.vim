" Vim syntax file
" Language:	ChaiScript
" Maintainer:	Jason Turner <jason 'at' emptycrate com>

syn case match

" syncing method
syn sync fromstart

" Strings
syn region chaiscriptString  start=+"+ end=+"+ skip=+\\\\\|\\"+ contains=chaiscriptSpecial,@Spell


" integer number
syn match chaiscriptNumber "\<\d\+\>"
" floating point number, with dot, optional exponent
syn match chaiscriptFloat  "\<\d\+\.\d*\%(e[-+]\=\d\+\)\=\>"
" floating point number, starting with a dot, optional exponent
syn match chaiscriptFloat  "\.\d\+\%(e[-+]\=\d\+\)\=\>"
" floating point number, without dot, with exponent
syn match chaiscriptFloat  "\<\d\+e[-+]\=\d\+\>"

syn match chaiscriptNumber "\<0x\x\+\>"

syn keyword chaiscriptCond if else

syn keyword chaiscriptRepeat while for do
syn keyword chaiscriptStatement break continue return

syn keyword chaiscriptExceptions try catch throw

"Keyword
syn keyword chaiscriptKeyword def true false attr

syn keyword chaiscriptType fun var

"Built in funcs
syn keyword chaiscriptFunc eval throw

"Let's treat all backtick operator function lookups as built in too
syn region chaiscriptFunc  matchgroup=chaiscriptFunc start="`" end="`"

syn match chaiscriptOperator "\.\."

" Comments
syn match   chaiscriptComment          "//.*$" contains=@Spell
syn region  chaiscriptComment        matchgroup=chaiscriptComment start="/\*" end="\*/" contains=@Spell



command -nargs=+ HiLink hi def link <args>

HiLink chaiscriptExceptions     Exception
HiLink chaiscriptKeyword        Keyword
HiLink chaiscriptStatement	Statement
HiLink chaiscriptRepeat		Repeat
HiLink chaiscriptString		String
HiLink chaiscriptNumber		Number
HiLink chaiscriptFloat		Float
HiLink chaiscriptOperator	Operator
HiLink chaiscriptConstant	Constant
HiLink chaiscriptCond		Conditional
HiLink chaiscriptFunction	Function
HiLink chaiscriptComment	Comment
HiLink chaiscriptTodo		Todo
HiLink chaiscriptError		Error
HiLink chaiscriptSpecial	SpecialChar
HiLink chaiscriptFunc		Identifier
HiLink chaiscriptType           Type

delcommand HiLink

let b:current_syntax = "chaiscript"

" vim: nowrap sw=2 sts=2 ts=8 noet ff=unix:
