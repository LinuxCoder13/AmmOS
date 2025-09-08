int TIME 0
int TIME_COOLDOWN 4

loop inf
    sleep $TIME_COOLDOWN
    reval TIME += $TIME_COOLDOWN
endloop
