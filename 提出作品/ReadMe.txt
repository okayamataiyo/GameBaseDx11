ゲーム説明
2匹の犬を操作してそれぞれのしたい目標の手助けをしよう！片方の犬は骨を集めるために走りまわる！もう片方の犬は構ってほしくて骨を集める犬の邪魔をする！片方がスコア150点になったらゲーム終了!

・実行方法・操作方法
キーボード・マウス	K&M
コントローラー		■
タイトルシーン:
セレクトシーンへ移行	K&M:Eキー,マウス左クリック	■:Aボタン
セレクトシーン:
説明書表示					■:Yボタン
2画面に変移(右画面出現)	K&M:Yキー
左画面だけに変移	K&M:Uキー
右画面だけに変移	K&M:Tキー
2画面に変移(左画面出現)	K&M:Rキー
ゲームシーンへ移動	K&M:Eキー,マウス左クリック	■:Aボタン
ゲームシーン:
プレイヤー共通の動き:
移動						■:左スティック
ジャンプ					■:Aボタン
飛びつき					■:RTボタン
ダッシュ					■:Rボタン
視点移動					■:右スティック
自由視点					■:Lボタン+右スティック
邪魔する側:
木箱を呼び出す					■:Yボタン
ゲームオーバーシーン:
タイトルシーンへ移行　	K&M:Eキー,マウス左クリック	■:Aボタン

・アピールポイント
画面分割を利用した2人プレイとプレイヤーの動きを時間をかけて作成しました！
特にプレイヤーの地面の坂を上り下りできて、ジャンプや自由落下も再現できるようにしました。Player.cppのPlayerRayCast()関数とPlayerMove()関数内の処理を見てほしいです！
また、Direct3D.cppのUpdate()関数内のビューポートの処理も工夫して作成しました！


〇フォルダ構成
[Dog_Big_Fight]
├─プレイ動画----提出作品のプレイ動画ファイルが格納されています。
├─プロジェクト一式----提出作品のソースコードファイルが格納されています。
│  ├─Assets----提出作品のモデル、画像を入れているファイルが格納されています。
│  │  ├─DebugCollision----デバック用コリジョンが格納されています。
│  │  └─Shader----シェーダー関連のソースファイルが格納されています。
│  ├─Engine----提出作品のゲームエンジンファイルが格納されています。
│  │  └─ImGui----ImGui用ファイルが格納されています。
│  ├─Object----提出作品のオブジェクトファイルが格納されています。
│  ├─Player----提出作品のプレイヤーファイルが格納されています。
│  └─Scene----提出作品のシーンファイルが格納されています。
└─実行ファイル----実行可能ファイルが格納されています。
    ├─Debug----デバックの実行ファイルが格納されています。
    │  └─Assets----モデル、画像ファイルが格納されています。
    │      ├─DebugCollision----デバック用コリジョンが格納されています。
    │      └─Shader----シェーダー関連のソースファイルが格納されています。
    └─Release----リリースの実行ファイルが格納されています。
        └─Assets----リリースの実行ファイルが格納されています。
            ├─DebugCollision----デバック用コリジョンが格納されています。
            └─Shader----シェーダー関連のソースファイルが格納されています。