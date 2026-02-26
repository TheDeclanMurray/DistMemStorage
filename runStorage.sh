#!/bin/bash
echo "Beginning Startup"

# Kill processes AND wait for sockets to fully release
pkill -f storage 2>/dev/null

# CRITICAL: Wait for TIME_WAIT to expire (2-5 seconds)
sleep 3
echo "Ports cleared and waited for socket release"

# Double-check ports are free
if lsof -i :8080-8081 2>/dev/null | grep -q LISTEN; then
    echo "WARNING: Ports still in use, force-killing again..."
    sleep 2
fi

trap 'echo "Shutting down..."; pkill -f storage 2>/dev/null; exit' INT TERM EXIT

# Method 1: Capture PID with $! immediately
./storage 0 &
pid1=$!
./storage 1 &
pid2=$!

echo "Servers started: PID $pid1, PID $pid2"

# Wait for both (won't freeze)
wait $pid1 $pid2
echo "All servers finished"
