from catterer/arrow_builder
COPY main.cc .
RUN g++ -O0 -g -std=c++2a main.cc -o arr-srv -lnghttp2_asio -lssl -lcrypto -lpthread
#CMD ["gdb", "-ex", "run", "--args", "./arr-srv", "0.0.0.0", "8080", "1", "/"]
CMD ["./arr-srv", "0.0.0.0", "8080", "1", "/"]
