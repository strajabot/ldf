FROM alpine:latest

RUN apk add --no-cache g++ cmake make catch2-3

WORKDIR /app

COPY . .

CMD ["sh", "-c", "cmake -B build && cmake --build build && ctest --test-dir build --output-on-failure"]
