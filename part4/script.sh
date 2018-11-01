#!/bin/bash

SESSION=$USER

tmux -2 new-session -d -s $SESSION

for i in $(seq 10)
do
        tmux new-window -n $i 
        tmux send-keys -t $SESSION:$i "ssh vdi-linux-0$((39+$i)).ccs.neu.edu" Enter
        tmux send-keys -t $SESSION:$i "cd src/dist-membership" Enter
        tmux send-keys -t $SESSION:$i "./prj2 -s -h hostfile -p 10000 -i $(($i-1)) 2>stderr.$i | tee stdout.$i" Enter
done

sleep 60

tmux send-keys -t $SESSION:5 C-c

sleep 10

tmux send-keys -t $SESSION:1 C-c

tmux attach-session -t $SESSION
