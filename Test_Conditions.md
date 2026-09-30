# Test Conditions

## Purpose
  
The purpose of this test is to evaluate scheduling characteristics, message processing performance, and real-time behavior of the system under various load conditions.

## Measurement Conditions
  
The evaluation is performed using combinations of the following conditions.

### Communication Conditions

- Number of topics that simultaneously Publish / Subscribe
- Data size of each topic
- Message transmission and reception frequency
- Network load
    - Configurable according to the number of topics and communication volume

### System Load Conditions

- CPU utilization
- Number of allocated CPU cores
- Task priority settings
- Execution conditions with mixed Timers of different periods
- Conditions where processing timing intentionally conflicts
- Conditions where processing timing does not conflict

### Middleware Conditions

- Type and configuration of RMW (ROS Middleware)

## Measurement Items

### Timing Characteristics

- Deviation of callback execution period from the target period
- Latency
- Time from Wait Set registration to dispatch start
- Histogram of each measurement timing
- Number of times processing did not complete within the specified period
- Number of message loss occurrences

### Resource Utilization

- CPU utilization
- Time-series changes in CPU utilization
- Maximum CPU utilization

### Throughput Characteristics

- Message processing rate
- Maximum number of processable messages under saturation conditions
- Queue backlog size
- Queue backlog time

### Quality and Reliability Metrics

- Message drop rate
- Overrun occurrence rate
    - Number of missed deadlines and message loss occurrences
- Frequency of priority inversion occurrences
