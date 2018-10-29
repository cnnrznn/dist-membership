#!/bin/bash

SESSION=$USER

tmux -2 new-session -d -s $SESSION

for i in $(seq 0 9)
do
        tmux new-window -t $SESSION:$i -n $i \
                "ssh -A $USER@vdi-linux-0$((50+$i)).ccs.neu.edu \
                ./prj2_tm -h hostfile -p 10000 -i $i"
done

tmux attach-session -t $SESSION

tmux kill-session -t $SESSION
