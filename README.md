## ROS2 Realtime Evaluation

This repository provides an evaluation environment for measuring the performance of ROS 2 Realtime Support features. On Ubuntu 24.04 LTS, Docker is used to collect and visualize measurement logs.

### Test Environment

This performance measurement program has been verified in the following environment.  
Ubuntu 24.04.4 LTS

### Prerequisites

#### Docker Installation (Optional)

If Docker is used, install Docker Engine in advance.
If Docker is already installed, this step can be skipped.  
Refer to the official Docker documentation for installation instructions:
<https://docs.docker.com/engine/install/>  

#### Installing a Real-Time Kernel

On Ubuntu 24.04 or later, install a Linux kernel with the real-time patch applied using the following command:

```bash
sudo apt install ubuntu-realtime
```
  
Reboot after the installation completes successfully:

```bash
sudo reboot
```

After rebooting, run the following command to verify that PREEMPT_RT is enabled:

```bash
uname -a | grep PREEMPT_RT
# Kernel information should be displayed when the command is executed
```

#### Clone the Source Code

Clone the source code of this repository.

```bash
git clone https://github.com/esol-community/ros2-realtime-evaluation.git
```

### Collecting Measurement Logs

Since collection is performed using docker compose, run the docker compuse run command:

```bash
docker compose run --build --remove-orphans record
```

### Using the `ros2_realtime_support` Package

When using `ros2_realtime_support`, you must load the real-time thread configuration file in advance.

```bash
export ROS_THREAD_ATTRS_FILE=realtime.yml
```

After running the command above, perform the [Collecting Measurement Logs](#collecting-measurement-logs). This ensures that the real-time package is correctly applied when `${EXECUTOR}=realtime_single` or `${EXECUTOR}=realtime_multi` is specified.

For details on the real-time thread configuration file, refer to [ros2_realtime_support/examples_rclcpp_realtime/README.md](https://github.com/esol-community/ros2_realtime_support/blob/rolling/examples_rclcpp_realtime/README.md#configuration-methods).

### Configuration of Experimental Conditions

This software allows the following experiment parameters to be configured:

- `executor`: Executor type to use
  - For the list of available executor types, see Available ROS [launch-arguments](#list-of-ros2-launch-arguments)
- `stress`: Whether to apply additional processing load.

`executor` and `stress` can also be specified using the `EXECUTOR` and `STRESS` arguments when executing the command above.

*(Default values: `${EXECUTOR}=single`, `${STRESS}=true`)*

```bash
STRESS=true EXECUTOR=realtime_single docker compose run --build --remove-orphans record
```

Measurement logs are stored in `logs/realtime_single_true/$(date +%Y%m%d%H%M%S)/`.

##### List of ros2 launch Arguments

The following arguments can be configured when executing ros2 launch.

- executor
- qos_reliability
- topic_size
- stress
- timeout
- topic_count_param
- publish_period_ms  

**executor**

- **Available values**:  single(default) / multi / events/　staic_single / cie / cbg / realtime_simgle / realtime_multi

- **Description**:   Argument specifying the Executor to use.  

The following table shows the mapping between the specified string and the applied Executor.

| Specified Value  | Implementation Class |
|----------------------|------------|
| `single`             | `rclcpp::executors::SingleThreadedExecutor` |
| `multi`              | `rclcpp::executors::MultiThreadedExecutor` |
| `events`             | `rclcpp::experimental::executors::EventsExecutor` |
| `static_single`      | `rclcpp::executors::StaticSingleThreadedExecutor` |
| `cbg`                | `rclcpp::executors::EventsCBGExecutor` |
| `cie`                | `CallbackIsolatedExecutor` |
| `realtime_single`    | `rclcpp_realtime::executors::SchedParamSingleThreadedExecutor` |
| `realtime_multi`     | `rclcpp_realtime::executors::SchedParamMultiThreadedExecutor` |

**publish_period_ms**

- **Available values**: 10 / 100(default) (unit: msec)
  This parameter specifies the message publish period in milliseconds. It is used to control the message transmission interval.

**qos_reliability**

- **Available values**: reliable(default) / best_effort
- **Description**:
  This parameter specifies the QoS (Quality of Service) Reliability setting for communication. Select `reliable` or `best_effort` to control the reliability of data delivery.

**stress**

- **Available values**: true / false(default)
- **Description**:
  This argument specifies whether stress test load is applied.

**timeout**

- **Available values**: 30.0(default) / any double value (unit: sec)

- **Description**:
  This parameter specifies the timeout period until processing is completed. If the specified time is exceeded, an error or termination process is executed.

**topic_size**

- **Available values**: 102400 (default), configurable up to the maximum value of `int`

- **Description**:
  This parameter specifies the size or data volume of the topic to be published or subscribed. It is used to configure message sizes during load testing.

**topic_count**

- **Available values**: 1 (default) / 10 / 50 / 100

- **Description**:
  This parameter specifies the number of publishers/subscribers to create or use. It is used for performance evaluation and operational verification in multi-topic environments.

##### When Using `CallbackIsoratedExecutor`

This package is also designed to support the use of `CallbackIsoratedExecutor`.  
The configuration file required for its use is provided at:  
`ros2_ws/src/sample_node/config/cie_settings.yaml`  
Modify the configuration as needed for your environment and requirements.  

### JupyterLab Visualization

The following section explains how to visualize the logs collected in `logs/`.  

Before visualization, run the following command to change the directory permissions so that the Docker environment can write to and execute files.  

```bash
chmod 755 ./caret_sample
chmod 755 ./caret_sample/config
chmod 777 ./caret_sample/jupyter
```

Start JupyterLab with the following command:

```bash
docker compose up --build visualize
```

Opening the displayed URL and running `caret.ipynb` will visualize the latest logs.  
If you only want to visualize the measurement logs, perform batch conversion with the following command:  

Set `CARET_LOG_NAME` to the name of the experiment-condition directory under `logs/` that you want to visualize. The following example visualizes `realtime_single_true`.  

```bash
CARET_LOG_NAME=realtime_single_true docker compose up --build visualize-to-html
```

If successful, html and ipynb files are generated under `caret_sample/jupyter/` using `nbconvert`.
In the command example above, the latest measurement log under `logs/realtime_single_true/` is visualized, and `realtime_single_true.html` and `realtime_single_true.ipynb` are generated in `caret_sample/jupyter/`.  
Since the generated files are overwritten, renaming or backing up files is required when collecting data repeatedly.  

For a sample of the experimentally measured results, refer to `caret_sample/jupyter/record_sample.html`.  
This sample demonstrates the execution results obtained using the default environment (`rclcpp::executors::SingleThreadedExecutor`).  

### Future Work

- Organize log outputs into separate directories for each executor and experimental condition.
- Add support for time-series collection of CPU and memory usage metrics.

## Acknowledgement

This work was supported by the New Energy and Industrial Technology Development Organization (NEDO), Japan, under commissioned research project JPNP25016.
