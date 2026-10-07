# ROS2 Realtime Evaluation

本リポジトリは、ROS 2のRealtime Support機能の性能評価を目的とした検証環境を提供する。Ubuntu 24.04 LT上で、Dockerを利用して計測ログの収集と可視化を行う。

## 検証環境

この性能計測プログラムは以下の環境で動作確認した。

Ubuntu 24.04.4 LTS


## 事前準備

### Dockerのインストール(optional)

Dockerを使用する場合は、事前にDocker Engineをインストールする。  
既にインストール済みの場合は、本手順を省略できる。  

インストール方法はDocker公式ドキュメント参照:  
<https://docs.docker.com/engine/install/>  

### リアルタイムカーネルの導入

Ubuntu 24.04以上で以下のコマンドを使いリアルタイムパッチを適用したLinuxカーネルをインストールする:

```bash
sudo apt install ubuntu-realtime
```

インストール成功後に再起動する:

```bash
sudo reboot
```

再起動後、以下のコマンドを実行し `PREEMPT_RT` の有効化を確認する:

```bash
uname -a | grep PREEMPT_RT
#コマンド実行時にカーネル情報が出力される事
```


### ソースコードのクローン

本リポジトリのソースコードをクローンする

```bash
git clone https://github.com/esol-community/ros2-realtime-evaluation.git
```

## 計測ログの収集

docker composeを使って収集を行うため `docker compuse run` コマンドで収集する:

```bash
docker compose run --build --remove-orphans record
```

正常に完了した場合は`logs/`以下にログが記録される。  

### docker-composeでの設定例

[docker-compose.yml](docker-compose.yml) の26行目のros2 launchに渡す引数を編集し、実験条件を設定する

以下はSingleThredExecutorを使用し、cpu負荷を与える場合の設定例である。

```yaml:docker-compose.yml
 ros2 launch sample_node trace_sample_node.launch.py stress:=true executor:=single 
```

また、executorについては、前述のコマンド実行時に`EXECUTOR`引数で指定することも可能である。  

```bash
EXECUTOR=single docker compose run --build --remove-orphans record
```

#### ros2 launchで設定可能な引数一覧

以下にros2 launch実行時に設定可能な引数を示す

- `executor` 
- `qos_reliability` 
- `topic_size` 
- `stress` 
- `timeout` 
- `topic_count_param` 
- `publish_period_ms` 

**executor**

- **設定可能値**：  single(default) / multi / events/　staic_single / cie / cbg / realtime_simgle / realtime_multi

- **説明**：   使用するExecutorを指定する引数

以下に指定する文字列と適用されるExecutorの対応を示す。

| 指定値  | 実体クラス |
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

- **設定可能値**：10 / 100(default) (単位:msec)
  メッセージのPublish周期をミリ秒単位で指定するパラメータである。送信間隔を制御するために使用する。

**qos_reliability**

- **設定可能値**： reliable(default) / best_effort
- **説明**：
  通信におけるQoS（Quality of Service）のReliability設定を指定するパラメータである。`reliable` または `best_effort` などを選択し、データ配信の信頼性を制御する。

**stress**

- **設定可能値**： true / false(default)
- **説明**：
  ストレス試験の負荷有無を指定する引数である。

**timeout**

- **設定可能値**： 30.0(default) / double値範囲 (単位:sec)

- **説明**：
  処理完了までのタイムアウト時間を指定するパラメータである。指定時間を超過した場合にエラーや終了処理を実行する。

**topic_size**

- **設定可能値**： 102400 (default) int最大値まで設定可能

- **説明**：
  パブリッシュまたはサブスクライブするトピックのサイズやデータ量を指定するパラメータである。負荷試験時のメッセージサイズ設定に利用する。

**topic_count**

- **設定可能値**： 1 (default) / 10 / 50 / 100

- **説明**：
  作成または使用するpublisher/subscriberの数を指定するパラメータである。複数トピック環境での性能評価や動作確認に利用する。

#### CallbackIsoratedExecutor使用時

本パッケージでは`CallbackIsoratedExecutor`を使用することも想定している。  
使用に必要な設定ファイルは、`ro2_ws/src/sample_node/config/cie_settings.yaml`に用意している。  
必要があれば適宜編集すること。  

## JupyterLabを使った可視化

`logs/`以下に収集したログを可視化する手順を説明する。  

docker環境から書き込み/実行できるよう、可視化前に以下のコマンドを実行して、ディレクトリのパーミッションを変更する。  

```bash
chmod 755 ./caret_sample
chmod 755 ./caret_sample/config
chmod 777 ./caret_sample/jupyter
```

以下のコマンドでJupyterLabを起動する:

```bash
docker compose up --build visualize
```

表示されたURLを開くと `caret.ipynb` のあるディレクトリを参照しているため、 `caret.ipynb` を開き実行することで最新ログをもとに可視化される。

また、計測ログを可視化するのみの場合は以下のコマンドで一括変換を行う:

```bash
docker compose up --build visualize-to-html
```

成功すると `nbconvert` を使い `caret_sample/jupyter/` 下にhtmlとipynbが生成される。  
生成されるファイルは上書きされるため、連続してデータ取得を行う際にはファイルのリネームや退避の作業が必要である。  

試験的に測定した結果のサンプルは `caret_sample/jupyter/record_sample.html` を参照すること。  
本サンプルは、デフォルト環境(rclcpp::executors::SingleThreadedExecutor)での動作結果となる。  

---

## Future Work

- ログファイルの出力先を、executorや実験条件毎に分けて出力する
- CPU/メモリ使用状況の時系列取得

## Acknowledgement

本研究は、国立研究開発法人 新エネルギー・産業技術総合開発機構（NEDO） の委託研究プロジェクト JPNP25016 の支援を受けて実施されました。
