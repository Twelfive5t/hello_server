#!/bin/bash
docker compose -p common_program -f docker-compose.yaml -f docker-compose.telemetry.yaml up -d
