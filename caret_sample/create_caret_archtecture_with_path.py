from caret_analyze import Architecture

arch = Architecture('lttng', '/home/user/logs/record_stress_non_rt/20260515013525/caret/e2e_sample/')

paths = arch.search_paths('/sensor_proc_1',
                          '/sensor_fusion')
type(paths) # list of multiple paths
paths[0].summary.pprint() # shows nodes and topics in paths[0]
arch.add_path('sensor_1_path', paths[0])

paths = arch.search_paths('/sensor_proc_2',
                          '/sensor_fusion')
type(paths) # list of multiple paths
paths[0].summary.pprint() # shows nodes and topics in paths[0]
arch.add_path('sensor_2_path', paths[0])

arch.export('/home/cross-test/mel-rospf-evaenv/caret_architecture_with_path.yaml', force=True)
