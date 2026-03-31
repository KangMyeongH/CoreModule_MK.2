# CoreModule

## 소개 | Overview

`CoreModule`은 Windows 전용 C++ / DirectX 11 기반 엔진 모듈입니다.  
`CoreModule` is a Windows-only engine module built with C++ and DirectX 11.

이 프로젝트는 "유니티의 API와 LifeCycle만 보고 내부 엔진은 어떻게 동작할지 상상하면서 직접 구현해보자"는 목표에서 시작했습니다.  
This project started from a simple goal: "If I only look at Unity's API and LifeCycle, how might the engine internals work?"

이 저장소는 독립 실행형 게임이 아니라 DLL 형태의 코어 모듈을 빌드합니다.  
This repository does not build a standalone game executable; it builds a DLL-based core module.

## 목표 | Goals

- 유니티와 유사한 엔진 사용 흐름을 직접 재구성  
  Reconstruct a Unity-like engine workflow
- `GameObject / Component / Scene / Prefab` 스타일 프로그래밍 모델 구현  
  Implement a `GameObject / Component / Scene / Prefab` style programming model
- 다중 패스를 포함한 DirectX 11 렌더링 파이프라인 구현  
  Implement a DirectX 11 rendering pipeline with multiple passes
- 런타임과 에디터 지향 기능을 같은 모듈 안에서 실험  
  Explore both runtime and editor-oriented workflows inside one module

## 구현 범위 | What Is Implemented

- 유니티와 유사한 코어 프레임 흐름  
  Unity-like core frame flow
  - `Initialization -> Physics -> InputEvent -> GameLogic -> SceneRender -> Decommissioning`
- 오브젝트 모델  
  Object model
  - `Object`, `GameObject`, `Component`, `Transform`, `Behaviour`, `ScriptBehaviour`
- 씬 시스템  
  Scene system
  - JSON 기반 씬 직렬화/역직렬화  
    JSON-based scene serialization and loading
  - 프리팹 직렬화 및 복제  
    Prefab serialization and cloning
- 렌더링  
  Rendering
  - `D3D11Manager`, `RenderManager`, render target, MRT 구성  
    `D3D11Manager`, `RenderManager`, render targets, and MRT setup
  - Base / Lighting / Deferred / Outline / Glow / Final pass  
    Base / Lighting / Deferred / Outline / Glow / Final passes
  - 머티리얼 및 셰이더 리플렉션 기반 바인딩  
    Material and shader reflection-based binding
- 물리 / 충돌  
  Physics / collision
  - Rigidbody 및 다양한 Collider 타입  
    Rigidbody and multiple collider types
  - broad phase / narrow phase 충돌 처리 흐름  
    Broad phase / narrow phase collision flow
- UI  
  UI
  - Texture UI, Text UI, 정렬 순서 기반 렌더링  
    Texture UI, Text UI, sorting-based rendering
  - DirectXTK `SpriteBatch` / `SpriteFont` 연동  
    DirectXTK `SpriteBatch` / `SpriteFont` integration
- 에디터 지원  
  Editor-side support
  - `EditorCore`, `Hierarchy`, `EditorComponentManager`, editor camera

## 저장소 구조 | Repository Layout

```text
CoreModule_MK.2-main/
|- CoreModule/                 # 메인 DLL 프로젝트 | Main DLL project
|  |- include/                # 공개 헤더 | Public headers
|  `- src/                    # 엔진 구현 | Engine implementation
|- ThirdParty/DirectXTK/      # 포함된 DirectXTK 헤더/라이브러리 | Checked-in DirectXTK headers/libs
|- Dist/                      # 배포용 헤더/바이너리 | Exported headers and binaries
|- bin/                       # 빌드 출력물 | Build outputs
|- packages/                  # NuGet 복원 산출물 | NuGet restore artifacts
`- CoreModule.sln             # Visual Studio 솔루션 | Visual Studio solution
```

## 주요 모듈 | Main Modules

### 1. Core / Engine Flow

- `Core`
- `TimeManager`
- `InputManager`
- `Scene`

프레임 진행 순서를 제어하는 런타임 진입 계층입니다.  
This is the runtime entry layer that orchestrates the engine frame flow.

### 2. Object Model

- `Object`
- `GameObject`
- `Component`
- `Transform`
- `Behaviour`
- `ScriptBehaviour`

유니티 스타일의 게임플레이 API에 가장 가까운 계층입니다.  
This is the gameplay-facing layer closest to a Unity-style API.

### 3. Rendering

- `D3D11Manager`
- `RenderManager`
- `RenderTarget`
- `Material`
- `Renderer`, `MeshRenderer`, `SkinnedMeshRenderer`
- `BasePass`, `LightingPass`, `DeferredPass`, `OutLinePass`, `GlowPass`, `FinalPass`

현재 프로젝트에서 가장 많은 구현이 쌓여 있는 축입니다.  
This is currently the strongest and most substantial part of the project.

### 4. Physics / Collision

- `PhysicsManager`
- `CollisionManager`
- `Rigidbody`
- `BoxCollider`, `SphereCollider`, `CapsuleCollider`, `MeshCollider`

Collider 등록, broad-phase 후보 생성, narrow-phase 판정, 충돌 콜백을 담당합니다.  
This layer handles collider registration, broad-phase candidate generation, narrow-phase checks, and collision callbacks.

### 5. Resource / Serialization

- `LoadManager`
- `PrefabManager`
- `Prefab`
- `AssetManager`

씬, 프리팹, 머티리얼, 모델, 애니메이션 직렬화를 담당합니다.  
This layer handles scene, prefab, material, model, and animation serialization.

### 6. Editor

- `EditorCore`
- `Hierarchy`
- `EditorCamera`
- `EditorComponentManager`

런타임과 분리된 에디터용 렌더링 및 씬 조작 계층입니다.  
This layer provides editor-facing rendering and scene management separated from runtime flow.

## 빌드 | Build

### 요구 사항 | Requirements

- Windows
- Visual Studio 2022
- MSVC v143 toolset
- Windows 10 SDK

### 의존성 | Dependencies

- DirectX 11
- DirectXTK
- `nlohmann/json`  
  `CoreModule/include/json.hpp`에 포함되어 있습니다.  
  Vendored under `CoreModule/include/json.hpp`.

DirectXTK는 `ThirdParty/DirectXTK` 아래에 포함되어 있고, 프로젝트는 `packages/`의 NuGet 복원 산출물도 참조합니다.  
DirectXTK is included under `ThirdParty/DirectXTK`, and the project also references the NuGet restore output in `packages/`.

### Visual Studio로 빌드 | Build With Visual Studio

1. `CoreModule.sln` 열기  
   Open `CoreModule.sln`
2. `Debug|x64` 또는 `Release|x64` 선택  
   Select `Debug|x64` or `Release|x64`
3. 솔루션 빌드  
   Build the solution

### MSBuild로 빌드 | Build With MSBuild

```powershell
msbuild .\CoreModule.sln /p:Configuration=Debug /p:Platform=x64 /t:Build /m:1 /nologo /v:minimal
```

### 빌드 출력 | Build Output

- `bin/Debug/CoreModule.dll`
- `bin/Debug/CoreModule.lib`
- `Dist/include/*`
- `Dist/bin/*`

Post-build 단계에서 공개 헤더와 바이너리가 `Dist/`로 복사됩니다.  
The post-build step copies exported headers and binaries into `Dist/`.

## 사용 전제 | Usage Notes

- 이 저장소는 실행 파일이 아니라 DLL을 빌드합니다.  
  This repository builds a DLL, not a standalone executable.
- 일부 리소스 경로는 현재 상위/형제 프로젝트 구조를 가정하고 하드코딩되어 있습니다.  
  Some resource paths are currently hardcoded and assume a sibling project layout.
  - `..\Client\Assets\...`
  - `..\GameEngine\resource\Shader\...`
- 그래서 이 저장소는 단독 클론보다, 더 큰 로컬 작업공간 안의 엔진 모듈로 사용할 때 가장 자연스럽습니다.  
  Because of those assumptions, this repository is easiest to use as a module inside a larger local workspace rather than as an isolated clone.

## 설계 의도 | Design Intention

이 프로젝트는 상용 엔진을 목표로 한 것이 아니라, 엔진 구조를 이해하기 위한 학습용 프로젝트로 시작했습니다.  
This project started as an engine study project rather than a production-ready engine.

핵심 질문은 다음과 같습니다.  
The core questions were:

- 유니티는 왜 이런 API 형태를 가지는가  
  Why does Unity expose this kind of API surface?
- 그 API를 뒷받침하려면 내부에 어떤 시스템이 필요한가  
  What kind of internal systems are required to support that API?
- 렌더링, 씬 관리, 에디터, 게임플레이 인터페이스는 어떻게 연결되는가  
  How do rendering, scene management, editor tooling, and gameplay-facing interfaces connect?

## 강점 | Current Strengths

- 개인 엔진 학습 프로젝트로서는 범위가 넓습니다.  
  Strong scope for a solo engine study project.
- 런타임, 렌더링, 물리, UI, 에디터를 분리하려는 시도가 분명합니다.  
  There is a clear attempt to separate runtime, rendering, physics, UI, and editor concerns.
- 유니티 스타일 사용 경험을 네이티브 C++에서 직접 복원해보려는 방향성이 뚜렷합니다.  
  It clearly explores a Unity-like usage model in native C++.
- 최소 샘플 수준을 넘는 DirectX 11 렌더링 파이프라인 구현이 들어 있습니다.  
  It contains meaningful DirectX 11 rendering pipeline work beyond a minimal sample.

## 한계 | Current Limitations

- 싱글턴과 전역 매니저 결합이 강합니다.  
  Heavy singleton/global-manager coupling.
- 일부 큰 클래스가 너무 많은 책임을 가지고 있습니다.  
  Some large classes still own too many responsibilities.
- 런타임과 에디터의 씬/프리팹 동작 일관성이 완전하지 않습니다.  
  Runtime/editor parity is incomplete in scene and prefab flows.
- 외부 리소스 경로 하드코딩이 남아 있습니다.  
  Hardcoded asset/shader paths remain.
- 자동화 테스트와 CI가 없습니다.  
  There is no automated test or CI pipeline yet.
- 현재 상태는 완성된 엔진보다는 엔진 프로토타입에 가깝습니다.  
  The repository is currently closer to an engine prototype than a production-ready framework.

## 다음 단계 | Recommended Next Steps

- 씬/프리팹 로딩 계약 안정화  
  Stabilize scene and prefab loading contracts
- 런타임/에디터 렌더링 중복 축소  
  Reduce duplicated runtime/editor rendering flow
- 큰 매니저 클래스 분리  
  Refactor large manager classes into smaller subsystems
- 하드코딩된 경로 제거  
  Remove hardcoded external resource paths
- 최소 실행 샘플 또는 데모 추가  
  Add a minimal host sample or demo executable
- README 이미지, 영상, 기능 체크리스트 추가  
  Add README images, demo video links, and a feature checklist

## 프로젝트 상태 | Project Status

현재 상태는 `active prototype / study project` 입니다.  
Current status: `active prototype / study project`.

이 저장소를 가장 정확하게 설명하는 문장은 다음과 같습니다.  
The most accurate one-line description of this repository is:

> 유니티의 표면 API와 LifeCycle을 보고 내부 구조를 역으로 상상하며 구현한 C++ / DirectX 11 엔진 프로토타입  
> A C++ / DirectX 11 engine prototype built by reverse-engineering Unity's surface API and life cycle into a self-designed internal architecture
