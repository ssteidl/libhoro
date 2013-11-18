set files [list doorbell.h cron.h Parser.h Parser.c cron.c lex.horo.c \
               doorbell.c]

set amalout [open "dbell-amal.c" [list WRONLY CREAT]];

foreach file $files {

    set fd [open $file RDONLY]
    fcopy $fd $amalout
    close $fd
}

close $amalout
