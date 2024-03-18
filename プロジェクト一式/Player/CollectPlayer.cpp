#include "../Engine/SceneManager.h"
#include "../Engine/Input.h"
#include "../Engine/Model.h"
#include "../Engine/Direct3D.h"
#include "../Engine/ImGui/imgui.h"
#include "../Engine/Text.h"
#include "CollectPlayer.h"
#include "AttackPlayer.h"
#include "../Stage.h"
#include "../Object/Floor.h"
#include "../Object/WoodBox.h"
CollectPlayer::CollectPlayer(GameObject* _pParent)
    :PlayerBase(_pParent, collectPlayerName), hModel_{-1},stageHModel_(-1),floorHModel_(-1), number_(0),playerState_(PLAYERSTATE::WAIT), playerStatePrev_(PLAYERSTATE::WAIT), gameState_(GAMESTATE::READY)
    , pParent_(nullptr), pPlayScene_(nullptr), pAttackPlayer_(nullptr), pCollision_(nullptr), pWoodBox_(nullptr), pText_(nullptr),pStage_(nullptr),pFloor_(nullptr)
{
    pParent_ = _pParent;
    timeCounter_ = 0;
    score_ = 0;
    positionPrev_ = { 0.0f,0.0f,0.0f };
    controllerMoveSpeed_ = 0.3f;
    mouseMoveSpeed_ = 0.3f;
    positionY_ = 0.0f;
    isDash_ = false;
    isFling_ = 1.0f;
    vecMove_ = { 0.0f,0.0f,0.0f,0.0f };
    vecLength_ = { 0.0f,0.0f,0.0f,0.0f };
    vecFront_ = { 0.0f,0.0f,0.0f,0.0f };
    vecDot_ = { 0.0f,0.0f,0.0f,0.0f };
    vecCross_ = { 0.0f,0.0f,0.0f,0.0f };
    length_ = 0.0f;
    dot_ = 0.0f;
    angle_ = 0.0f;
    positionTempY_ = 0.0f;
    positionPrevY_ = 0.0f;
    isJump_ = false;
    isOnFloor_ = false;
    woodBoxName_ = "WoodBox";
    woodBoxNumber_ = "WoodBox0";
    dotProduct_ = 0.0f;
    angleDegrees_ = 0.0f;
    stunTimeCounter_ = 0;
    stunLimit_ = 0;
    isStun_ = false;
    isKnockBack_ = false;
    rayFloorDistUp_ = 0.0f;
    rayFloorDistDown_ = 0.0f;
    rayStageDistDown_ = 0.0f;
    rayStageDistFront_ = 0.0f;
    rayStageDistBack_ = 0.0f;
    rayStageDistLeft_ = 0.0f;
    rayStageDistRight_ = 0.0f;
}

CollectPlayer::~CollectPlayer()
{
}

void CollectPlayer::Initialize()
{
    //モデルデータのロード
    std::string ModelName = collectPlayerName + (std::string)".fbx";
    hModel_ = Model::Load(ModelName);
    assert(hModel_ >= 0);
    transform_.scale_ = { 0.5,0.5,0.5 };
    positionY_ = transform_.position_.y;
    pCollision_ = new SphereCollider(XMFLOAT3(0.0f, 0.0f, 0.0f), 1.0f);
    AddCollider(pCollision_);
    pPlayScene_ = (PlayScene*)FindObject("PlayScene");
    pStage_ = (Stage*)FindObject("Stage");      //ステージオブジェクト
    pFloor_ = (Floor*)FindObject("Floor");
    pText_ = new Text;
    pText_->Initialize();
}

void CollectPlayer::Update()
{
    switch (gameState_)
    {
    case GAMESTATE::READY:          UpdateReady();      break;
    case GAMESTATE::PLAY:           UpdatePlay();       break;
    case GAMESTATE::GAMEOVER:       UpdateGameOver();   break;
    }
}

void CollectPlayer::Draw()
{
    pText_->Draw(30, 60, "CollectPlayer:Score=");
    pText_->Draw(360, 60, score_);

    Model::SetTransform(hModel_, transform_);
    Model::Draw(hModel_);
}

void CollectPlayer::Release()
{
}

void CollectPlayer::UpdateReady()
{
    ++timeCounter_;
    if (timeCounter_ >= 60)
    {
        gameState_ = GAMESTATE::PLAY;
        timeCounter_ = 0;
    }
}

void CollectPlayer::UpdatePlay()
{
    if (playerStatePrev_ != playerState_)
    {
        switch (playerState_)
        {
        case PLAYERSTATE::WAIT:       Model::SetAnimFrame(hModel_, 0, 0, 1.0f); break;
        case PLAYERSTATE::WALK:       Model::SetAnimFrame(hModel_, 20, 60, 0.5f); break;
        case PLAYERSTATE::RUN:        Model::SetAnimFrame(hModel_, 80, 120, 0.5f); break;
        case PLAYERSTATE::JUMP:       Model::SetAnimFrame(hModel_, 120, 120, 1.0f); break;
        case PLAYERSTATE::STUN:       Model::SetAnimFrame(hModel_, 140, 200, 0.5f); break;
        }
    }
    playerStatePrev_ = playerState_;
    PlayerRayCast();
    PlayerKnockback();
    transform_.position_.y = positionY_;
    if (isStun_)
    {
        stunTimeCounter_++;
        if (stunTimeCounter_ >= stunLimit_)
        {
            gameState_ = GAMESTATE::PLAY;
            isStun_ = false;
            isKnockBack_ = false;
            stunTimeCounter_ = 0;
        }
    }
    if (!isStun_)
    {
        PlayerMove();
    }
    if (score_ >= 150)
    {
        SceneManager* pSceneManager = (SceneManager*)FindObject("SceneManager");
        pSceneManager->ChangeScene(SCENE_ID_GAMEOVER);
        Direct3D::SetIsChangeView(1);
    }
    //ImGui::Text("playerState_=%i", playerState_);
    //ImGui::Text("positionPrevY_=%f", positionPrevY_);
    //ImGui::Text("positionTempY_=%f", positionTempY_);
    ImGui::Text("Transform_.position_.x=%f", transform_.position_.x);
    ImGui::Text("Transform_.position_.y=%f", transform_.position_.y);
    ImGui::Text("Transform_.position_.z=%f", transform_.position_.z);
    /*ImGui::Text("prevPosition_.x=%f", prevPosition_.x);
    ImGui::Text("prevPosition_.y=%f", prevPosition_.y);
    ImGui::Text("prevPosition_.z=%f", prevPosition_.z);*/
    //ImGui::Text("angleDegrees_=%f", angleDegrees_);
    //ImGui::Text("timeCounter_=%i", timeCounter_);
    if (IsMoving())
    {
        playerState_ = PLAYERSTATE::WALK;
    }
    else if (!isJump_)
    {
        playerState_ = PLAYERSTATE::WAIT;
    }
    if (Input::IsKey(DIK_LSHIFT) && !isJump_)
    {
        playerState_ = PLAYERSTATE::RUN;
        isDash_ = true;
    }
    else
    {
        isDash_ = false;
    }
    if (isJump_)
    {
        playerState_ = PLAYERSTATE::JUMP;
    }
    if (isStun_)
    {
        playerState_ = PLAYERSTATE::STUN;
    }
}

void CollectPlayer::UpdateGameOver()
{
    if (Input::IsKey(DIK_SPACE))
    {
        SceneManager* pSceneManager = (SceneManager*)FindObject("SceneManager");
        pSceneManager->ChangeScene(SCENE_ID_GAMEOVER);
    }
}

void CollectPlayer::Stun(int _timeLimit)
{
    //transform_.position_.y = positionY_;
    isStun_ = true;
    stunLimit_ = _timeLimit;
}

void CollectPlayer::OnCollision(GameObject* _pTarget)
{
    std::vector<int> woodBoxs = pPlayScene_->GetWoodBoxs();
    woodBoxNumber_ = woodBoxName_ + std::to_string(number_);
    if (_pTarget->GetObjectName() == woodBoxNumber_)
    {
        pWoodBox_ = (WoodBox*)FindObject(woodBoxNumber_);
        XMVECTOR vecPos = XMLoadFloat3(&transform_.position_) - pWoodBox_->GetVecPos();
        vecPos = XMVector3Normalize(vecPos);
        XMVECTOR vecUp = { 0,1,0,0 };
        dotProduct_ = XMVectorGetX(XMVector3Dot(vecPos, vecUp));
        float angleRadians = acosf(dotProduct_);
        angleDegrees_ = XMConvertToDegrees(angleRadians);
        if (angleDegrees_ <= 80)
        {
            PlayerJump();
            pWoodBox_->KillMe();
            pPlayScene_->AddPlayerFirstWoodBoxNum(-1);
        }
    }
    //WoodBoxという名前を持つ全てのオブジェクトの機能を実装
    if (_pTarget->GetObjectName().find("WoodBox") != std::string::npos)
    {
        if (angleDegrees_ > 80)
        {
            transform_.position_ = positionPrev_;
        }
    }
    if (_pTarget->GetObjectName().find("Bone") != std::string::npos)
    {
        score_ += 10;
        pPlayScene_->AddBoneCount(-1);
        _pTarget->KillMe();
    }
    ++number_;
    if (number_ >= woodBoxs.size())
    {
        number_ = 0;
    }
    //if (_pTarget->GetObjectName() == "PlayerFirst")
    //{
    //    SetGameState(GAMESTATE::FIRSTSTUN);
    //    XMVECTOR vecKnockbackDirection = -(XMLoadFloat3(&transform_.position_) - pPlayer_->GetVecPos());
    //    XMFLOAT3 moveRot = {};
    //    XMStoreFloat3(&moveRot, vecKnockbackDirection);
    //    moveRot.y = 0;
    //    vecKnockbackDirection = XMLoadFloat3(&moveRot);
    //    XMVECTOR vectorMove = XMLoadFloat3(&transform_.position_) + (vecKnockbackDirection / 4);
    //    XMStoreFloat3(&transform_.position_, vectorMove);
    //    pPlayer_->SetVecPos(-vectorMove);
    //    SetGameState(GAMESTATE::SECONDSSTUN);
    //}
    if (_pTarget->GetObjectName() == attackPlayerName)
    {
        Stun(10);
        isKnockBack_ = true;
    }
}

void CollectPlayer::PlayerMove()
{
    if (isDash_ == false)
    {
        //controllerMoveSpeed_ *= 0.9f;
    }
    else
    {
        //controllerMoveSpeed_ *= 1.1f;
    }
    if (!(Input::IsKey(DIK_F)))
    {
        XMVECTOR vecCam = {};
        vecCam = -(Camera::VecGetPosition(0) - Camera::VecGetTarget(0));
        XMFLOAT3 camRot = {};
        XMStoreFloat3(&camRot, vecCam);
        camRot.y = 0;
        vecCam = XMLoadFloat3(&camRot);
        vecCam = XMVector3Normalize(vecCam);
        vecMove_ = vecCam;
    }
    //向き変更
    vecLength_ = XMVector3Length(vecMove_);
    length_ = XMVectorGetX(vecLength_);
    if (length_ != 0)
    {
        //プレイヤーが入力キーに応じて、その向きに変える(左向きには出来ない)
        vecFront_ = { 0,0,1,0 };
        vecMove_ = XMVector3Normalize(vecMove_);

        vecDot_ = XMVector3Dot(vecFront_, vecMove_);
        dot_ = XMVectorGetX(vecDot_);
        angle_ = acos(dot_);

        //右向きにしか向けなかったものを左向きにする事ができる
        vecCross_ = XMVector3Cross(vecFront_, vecMove_);
        if (XMVectorGetY(vecCross_) < 0)
        {
            angle_ *= -1;
        }
    }

    transform_.rotate_.y = XMConvertToDegrees(angle_);
    if (Input::IsKey(DIK_W))
    {
        XMVECTOR vecDirection = XMLoadFloat3(&transform_.position_) - Camera::VecGetPosition(0);
        vecDirection = XMVectorSetY(vecDirection, 0);
        vecDirection = XMVector3Normalize(vecDirection);
        transform_.position_.x = transform_.position_.x + controllerMoveSpeed_ * XMVectorGetX(vecDirection);
        transform_.position_.z = transform_.position_.z + controllerMoveSpeed_ * XMVectorGetZ(vecDirection);
    }
    if (Input::IsKey(DIK_S))
    {
        XMVECTOR vecDirection = XMLoadFloat3(&transform_.position_) - Camera::VecGetPosition(0);
        vecDirection = XMVectorSetY(vecDirection, 0);
        vecDirection = XMVector3Normalize(vecDirection);
        transform_.position_.x = transform_.position_.x + controllerMoveSpeed_ * XMVectorGetX(-vecDirection);
        transform_.position_.z = transform_.position_.z + controllerMoveSpeed_ * XMVectorGetZ(-vecDirection);
    }
    if (Input::IsKey(DIK_D))
    {
        XMMATRIX rotmat = XMMatrixRotationY(3.14 / 2);                          //XMMatrixRotationY = Y座標を中心に回転させる行列を作る関数,//XMConvertToRadians = degree角をradian角に(ただ)変換する
        XMVECTOR vecDirection = XMLoadFloat3(&transform_.position_) - Camera::VecGetPosition(0);
        vecDirection = XMVectorSetY(vecDirection, 0);
        vecDirection = XMVector3Normalize(vecDirection);
        XMVECTOR tempvec = XMVector3Transform(vecDirection, rotmat);
        transform_.position_.x = transform_.position_.x + controllerMoveSpeed_ * XMVectorGetX(tempvec);
        transform_.position_.z = transform_.position_.z + controllerMoveSpeed_ * XMVectorGetZ(tempvec);
    }
    if (Input::IsKey(DIK_A))
    {
        XMMATRIX rotmat = XMMatrixRotationY(3.14 / 2);
        XMVECTOR vecDirection = XMLoadFloat3(&transform_.position_) - Camera::VecGetPosition(0);
        vecDirection = XMVectorSetY(vecDirection, 0);
        vecDirection = XMVector3Normalize(vecDirection);
        XMVECTOR tempvec = XMVector3Transform(vecDirection, -rotmat);
        transform_.position_.x = transform_.position_.x + controllerMoveSpeed_ * XMVectorGetX(tempvec);
        transform_.position_.z = transform_.position_.z + controllerMoveSpeed_ * XMVectorGetZ(tempvec);
    }
    if (Input::IsKeyDown(DIK_SPACE) && isJump_ == false)
    {
        PlayerJump();
    }
}

void CollectPlayer::PlayerJump()
{
    //ジャンプの処理
    isJump_ = true;
    positionPrevY_ = positionY_;
    positionY_ = positionY_ + 0.3;
}

void CollectPlayer::PlayerKnockback()
{
    if (isKnockBack_ == true)
    {
        XMVECTOR vecKnockbackDirection = (XMLoadFloat3(&transform_.position_) - pAttackPlayer_->GetVecPos());
        vecKnockbackDirection = XMVector3Normalize(vecKnockbackDirection);
        float knockbackSpeed = 0.3f;
        SetKnockback(vecKnockbackDirection, knockbackSpeed);
        Stun(30);
    }
}

void CollectPlayer::PlayerRayCast()
{
    RayCastData floorDataUp;
    RayCastData floorDataDown;
    RayCastData stageDataDownDown;
    RayCastData stageDataDownFront;
    RayCastData stageDataDownBack;
    RayCastData stageDataDownLeft;
    RayCastData stageDataDownRight;                 //プレイヤーが地面からどのくらい離れていたら浮いている判定にするか
    stageHModel_ = pStage_->GetModelHandle();         //モデル番号を取得
    floorHModel_ = pFloor_->GetModelHandle();
    if (isJump_ == true)
    {
        //放物線に下がる処理
        positionTempY_ = positionY_;
        positionY_ += (positionY_ - positionPrevY_) - 0.007;
        positionPrevY_ = positionTempY_;
        isJump_ = (positionY_ <= -rayFloorDistDown_ + 0.6f) ? false : isJump_;
        isJump_ = (positionY_ <= -rayStageDistDown_ + 0.6f) ? false : isJump_;
    }

    for (int i = 0; i <= 2; i++)
    {
        //▼上の法線(すり抜け床のため)
        floorDataUp.start = transform_.position_;           //レイの発射位置
        floorDataUp.dir = XMFLOAT3(0, 1, 0);                //レイの方向
        Model::RayCast(floorHModel_ + i, &floorDataUp);         //レイを発射
        rayFloorDistUp_ = floorDataUp.dist;
        //ImGui::Text("rayUpDist_=%f", rayUpDist_);

        //▼下の法線(すり抜け床)
        floorDataDown.start = transform_.position_;    //レイの発射位置
        floorDataDown.start.y = 0.0f;
        floorDataDown.dir = XMFLOAT3(0, -1, 0);        //レイの方向
        if (floorDataUp.dist == 99999)
        {
            Model::RayCast(floorHModel_ + i, &floorDataDown);  //レイを発射
        }
        rayFloorDistDown_ = floorDataDown.dist;
        //ImGui::Text("rayFloorDist_=%f", rayFloorDist_);
        if (rayFloorDistDown_ + positionY_ <= isFling_)
        {
            if (isJump_ == false)
            {
                positionY_ = -floorDataDown.dist + 0.6f;
                isOnFloor_ = 1;
                positionTempY_ = positionY_;
                positionPrevY_ = positionTempY_;
            }
        }
        else
        {
            isOnFloor_ = 0;
        }

    }
    //▼下の法線(床に張り付き)
    stageDataDownDown.start = transform_.position_;  //レイの発射位置
    stageDataDownDown.start.y = 0;
    stageDataDownDown.dir = XMFLOAT3(0, -1, 0);       //レイの方向
    Model::RayCast(stageHModel_, &stageDataDownDown); //レイを発射
    rayStageDistDown_ = stageDataDownDown.dist;
    //プレイヤーが浮いていないとき
    //ImGui::Text("rayGravityDist_=%f", rayGravityDist_);
    if (rayStageDistDown_ + positionY_ <= isFling_)
    {
        //ジャンプしてない＆すり抜け床の上にいない
        if (isJump_ == false && isOnFloor_ == 0)
        {
            //地面に張り付き
            positionY_ = -stageDataDownDown.dist + 0.6;
            positionTempY_ = positionY_;
            positionPrevY_ = positionTempY_;
        }
    }
    else if (isOnFloor_ == 0)
    {
        isJump_ = true;
    }
    //▼前の法線(壁の当たり判定)
    stageDataDownFront.start = transform_.position_;       //レイの発射位置
    stageDataDownFront.dir = XMFLOAT3(0, 1, 1);            //レイの方向
    Model::RayCast(stageHModel_, &stageDataDownFront);  //レイを発射
    rayStageDistFront_ = stageDataDownFront.dist;
    //ImGui::Text("rayStageDistFront_=%f", rayStageDistFront_);
    if (rayStageDistFront_ <= 1.5f)
    {
        //transform_.position_.z = (float)((int)prevPosition_.z) - 0.00001f;
        transform_.position_.z = positionPrev_.z;
    }
    //▼後ろの法線(壁の当たり判定)
    stageDataDownBack.start = transform_.position_;       //レイの発射位置
    stageDataDownBack.dir = XMFLOAT3(0, 1, -1);           //レイの方向
    Model::RayCast(stageHModel_, &stageDataDownBack);  //レイを発射
    rayStageDistBack_ = stageDataDownBack.dist;
    //ImGui::Text("rayStageDistBack_=%f", rayStageDistBack_);
    if (rayStageDistBack_ <= 1.5f)
    {
        //transform_.position_.z = (float)((int)prevPosition_.z) + 0.00001f;
        transform_.position_.z = positionPrev_.z;
    }
    //▼左の法線(壁の当たり判定)
    stageDataDownLeft.start = transform_.position_;       //レイの発射位置
    stageDataDownLeft.dir = XMFLOAT3(-1, 1, 0);           //レイの方向
    Model::RayCast(stageHModel_, &stageDataDownLeft);  //レイを発射
    rayStageDistLeft_ = stageDataDownLeft.dist;
    //ImGui::Text("rayStageDistLeft_=%f", rayStageDistLeft_);
    if (rayStageDistLeft_ <= 1.5f)
    {
        //transform_.position_.x = (float)((int)prevPosition_.x) + 0.00001f;
        transform_.position_.x = positionPrev_.x;
    }
    //▼右の法線(壁の当たり判定)
    stageDataDownRight.start = transform_.position_;       //レイの発射位置
    stageDataDownRight.dir = XMFLOAT3(1, 1, 0);           //レイの方向
    Model::RayCast(stageHModel_, &stageDataDownRight);  //レイを発射
    rayStageDistRight_ = stageDataDownRight.dist;
    //ImGui::Text("rayStageDistRight_=%f", rayStageDistRight_);
    if (rayStageDistRight_ <= 1.5f)
    {
        //transform_.position_.x = (float)((int)prevPosition_.x) - 0.00001f;
        transform_.position_.x = positionPrev_.x;
    }
    positionPrev_ = transform_.position_;
}

void CollectPlayer::SetKnockback(XMVECTOR _vecKnockbackDirection, float _knockbackSpeed)
{
    transform_.position_.x = transform_.position_.x + _knockbackSpeed * XMVectorGetX(_vecKnockbackDirection);
    transform_.position_.z = transform_.position_.z + _knockbackSpeed * XMVectorGetZ(_vecKnockbackDirection);
}

bool CollectPlayer::IsMoving()
{
    return (transform_.position_.x != positionPrev_.x || transform_.position_.z != positionPrev_.z);
}