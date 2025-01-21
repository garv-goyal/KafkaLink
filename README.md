# KafkaLink
KafkaLink is a Apache Kafka wire protocol exploration in C++. This project explores how Kafka brokers handle port binding, correlation IDs, API version checks, and more.

## Objectives
- To gain a deeper understanding of Kafka's internal mechanisms.
- To implement key features of the Kafka protocol, such as correlation IDs, API version checks, and topic metadata handling.
- To build a lightweight, educational broker implementation that handles basic client requests.

## Key Implementation Steps
1. **Connection Setup**
   - [x] Bind to a Port: Set up a TCP server on port 9092.
   - [ ] Concurrent Clients: Handle multiple client connections.

2. **Request Parsing**
   - [x] Send Correlation ID: Respond with a correlation ID.
   - [x] Parse Correlation ID: Extract correlation ID from requests.
   - [x] Parse API Version: Validate API versions and handle errors.

3. **API Handling**
   - [x] Handle ApiVersions Requests: Support ApiVersions requests.
   - [ ] Listing Partitions: Provide topic and partition metadata.

4. **Message Handling**
   - [ ] Consuming Messages: Fetch and return topic messages.
   - [ ] Fetch from Disk: Retrieve messages from storage.
  
## Setup and Run

1. **Clone & Build**
```
git clone https://github.com/your-username/KafkaLink.git
cd KafkaLink
g++ -o kafka_link main.cpp
```

2. **Execute**
```
./kafka_link
```

3. **Testing**
- Use tools like `nc` or `telnet` to simulate Kafka client requests.

### ⚠️ Currently working on this repository!