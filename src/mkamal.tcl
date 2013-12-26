#
# December 19, 2013
# The author disclaims copyright to this source code.
#

set amalFileName "horo-amal.c"

set files [list horo.h cron.h Parser.h Parser.c cron.c lex.horo.c \
               horo.c]

#Cat the files together
proc createAmal {} {
    global amalFileName
    global files

    set amalout [open $amalFileName [list WRONLY CREAT]];

    foreach file $files {

	set fd [open $file RDONLY]
	fcopy $fd $amalout
	close $fd
    }

    close $amalout
}

#Removes all of the "#line <num> <file>" macros since they
#no longer apply
proc removeLineDeclarations {} {
    global amalFileName

    set amalFD [open $amalFileName "r+"]
    set amalString [read $amalFD]
    set scrubbedString [regsub -expanded -all -line {\#line.*$} "$amalString" {}]
    close $amalFD

    set amalFD [open $amalFileName "w"]
    puts $amalFD $scrubbedString
    fconfigure $amalFD -encoding binary
    close $amalFD
}

createAmal
removeLineDeclarations