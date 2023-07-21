FROM alpine:latest AS build
RUN apk add --no-cache python3 py3-pip clang clang-dev musl-dev make
RUN pip install conan
RUN pip install cmake
WORKDIR /workspace
COPY . .
RUN conan profile detect
RUN conan install . --output-folder=build --build=missing
WORKDIR build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build .
RUN cmake --install . --prefix "/app"

FROM alpine:latest AS final
EXPOSE 80
RUN apk add --no-cache bash libstdc++
WORKDIR /app
COPY --from=build /app .
ENTRYPOINT ["./bin/DockerSample"]
