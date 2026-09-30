from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, TimerAction, Shutdown, ExecuteProcess
from launch.substitutions import LaunchConfiguration
from launch.conditions import IfCondition
from launch_ros.actions import Node
from tracetools_launch.action import Trace

def generate_launch_description():
    arg_executor = DeclareLaunchArgument('executor', default_value='single')
    arg_qos_reliability = DeclareLaunchArgument('qos_reliability', default_value='reliable')
    arg_topic_size = DeclareLaunchArgument('topic_size', default_value='102400')
    arg_stress = DeclareLaunchArgument('stress',default_value='false')
    arg_timeout = DeclareLaunchArgument('timeout', default_value='30.0')
    arg_topic_count = DeclareLaunchArgument('topic_count', default_value='1')
    arg_publish_period_ms = DeclareLaunchArgument('publish_period_ms',default_value='100')

    trace = Trace(
        session_name='samplenode',
        events_kernel=[],
        events_ust=['ros2*'],
    )

    node = Node(
        package='sample_node',
        executable='nodes',
        parameters=[
            {'topic_size': LaunchConfiguration('topic_size')},
            {'publish_period_ms': LaunchConfiguration('publish_period_ms')},
            {'qos_reliability': LaunchConfiguration('qos_reliability')},
            {'topic_count': LaunchConfiguration('topic_count')}
        ],
        arguments=[
            '--',
            '--executor', LaunchConfiguration('executor')
        ],
    )

    stress_process = ExecuteProcess(
        cmd=['stress-ng', '--cpu', '0', '--cpu-load', '50'],
        output='screen',
        condition=IfCondition(LaunchConfiguration('stress')) # Executability depends on the specified `stress` value.
    )

    stop_after_period = TimerAction(
        period=LaunchConfiguration('timeout'),
        actions=[Shutdown(reason="Measurement done")]
    )

    return LaunchDescription([
        arg_executor,
        arg_qos_reliability, 
        arg_topic_size,
        arg_stress,
        arg_timeout,
        arg_topic_count,
        arg_publish_period_ms,
        trace,
        node,
        stress_process,
        stop_after_period
    ])
