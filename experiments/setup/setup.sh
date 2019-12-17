# provide two arguments - server name, experiment name before testing like imgur
# setup routes
. ../config
bash assignlegitimate
bash assignattackers
ssh $1.$experiment_name.leader "bash $path_to_execute_from/experiments/setup/setup_server.sh $experiment_name $path_to_execute_from"
# setup attackers
bash tune-all.sh $1
