FROM gcc:latest

# Install tools
RUN apt-get update && apt-get install -y \
    iproute2 \
    net-tools \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Option A: Copy the 'router' folder from your computer into /app
COPY router/ .

# Now the Makefile is right here in /app, so make will work
RUN make clean && make 

# Run the router
CMD ["./router"]