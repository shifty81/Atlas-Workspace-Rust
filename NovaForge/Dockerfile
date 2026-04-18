# NovaForge — Dedicated server container
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake g++ git ca-certificates \
    libgl1-mesa-dev libglfw3-dev nlohmann-json3-dev \
    libfreetype-dev libopenal-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

RUN cmake -B Builds/Release \
    -DCMAKE_BUILD_TYPE=Release \
    -DNF_BUILD_EDITOR=OFF \
    -DNF_BUILD_GAME=OFF \
    -DNF_BUILD_SERVER=ON \
    -DNF_BUILD_TESTS=OFF \
    && cmake --build Builds/Release --target NovaForgeServer --parallel

# ── Runtime image ─────────────────────────────────────────────────
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 libopenal1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /novaforge
COPY --from=builder /build/Builds/Release/bin/NovaForgeServer .
COPY Data/ Data/
COPY Config/ Config/
COPY Schemas/ Schemas/

EXPOSE 27015/udp
EXPOSE 27015/tcp

# Server metadata
LABEL org.opencontainers.image.title="NovaForge Server" \
      org.opencontainers.image.description="NovaForge dedicated game server" \
      org.opencontainers.image.version="0.1.0"

ENTRYPOINT ["./NovaForgeServer"]
