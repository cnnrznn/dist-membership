#!/bin/bash

SESSION=$USER

tmux -2 new-session -d -s $SESSION

for i in $(seq 10)
do
        tmux new-window -n $i 
        tmux send-keys -t $SESSION:$i "ssh vdi-linux-0$((39+$i)).ccs.neu.edu" Enter
        tmux send-keys -t $SESSION:$i "cd src/dist-membership" Enter
        tmux send-keys -t $SESSION:$i "sleep $((i*5))" Enter
        tmux send-keys -t $SESSION:$i "./prj2 -h hostfile -p 10000 -i $(($i-1)) 2>stderr.$i | tee stdout.$i" Enter
done

tmux attach-session -t $SESSION

