#
# December 19, 2013
# The author disclaims copyright to this source code.
#

set files [list horo.h cron.h Parser.h Parser.c cron.c lex.horo.c \
               horo.c]

set amalout [open "horo-amal.c" [list WRONLY CREAT]];

foreach file $files {

    set fd [open $file RDONLY]
    fcopy $fd $amalout
    close $fd
}

close $amalout
