ARG DESTDIR='/artifacts'

FROM ubuntu:22.04 AS builder

ARG DEBIAN_FRONTEND='noninteractive' \
    BUILDDIR='/build' \
    DESTDIR

RUN apt-get update && apt-get upgrade && \
    apt-get -y install \
        libboost-container-dev libicu-dev libfuse-dev libzip-dev \
        g++ pkg-config make pandoc \
        python3

COPY --link . "${BUILDDIR}"
WORKDIR "${BUILDDIR}"

RUN make

RUN mkdir -v -p "${DESTDIR}" && make install-strip

FROM ubuntu:22.04 AS final

ARG DESTDIR

COPY --from=builder "${DESTDIR}" /

VOLUME "${DESTDIR}"
