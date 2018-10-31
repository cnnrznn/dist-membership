#!/bin/bash

SESSION=$USER

tmux -2 new-session -d -s $SESSION

for i in $(seq 10)
do
        tmux new-window -n $i 
        tmux send-keys -t $SESSION:$i "ssh vdi-linux-0$((39+$i)).ccs.neu.edu" Enter
        tmux send-keys -t $SESSION:$i "cd src/dist-membership" Enter
        tmux send-keys -t $SESSION:$i "./prj2 -h hostfile -p 10000 -i $(($i-1)) 2>stderr.$i | tee stdout.$i" Enter
done

sleep 30

for i in $(seq 9)
do
        tmux send-keys -t $SESSION:$((11-$i)) C-c
        sleep 5
done

tmux attach-session -t $SESSION
