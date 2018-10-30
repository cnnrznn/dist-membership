#!/bin/bash

SESSION=$USER

tmux -2 new-session -d -s $SESSION

for i in $(seq 10)
do
        tmux new-window -n $i 
        tmux send-keys -t $SESSION:$i "ssh vdi-linux-0$((29+$i)).ccs.neu.edu" Enter
        tmux send-keys -t $SESSION:$i "cd src/dist-membership" Enter
        tmux send-keys -t $SESSION:$i "./prj2 -h hostfile -p 10000 -i $(($i-1))" Enter
done

tmux attach-session -t $SESSION

