# Server
./picoquicdemo -c certs/cert.pem -k certs/key.pem -p 4433 -M -w ../downloadtest -F server_log.csv -l server_logs.txt

# Client
./picoquicdemo -l client_logs.txt -A 0.0.0.0/0 -q client_log.csv -a perf -M -o ../downloadtest localhost 4433 "*1:200:70000000"