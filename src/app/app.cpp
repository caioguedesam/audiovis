#include "app/app.hpp"
#include "engine/renderer/renderer.hpp"

#include "miniaudio.h"
#include "fft.hpp"

namespace Ty
{

//std::string srcVS_Default;
//std::string srcPS_Default;

//std::vector<Handle<MeshRenderable>> h_Renderables_Sponza;
//std::vector<Handle<MeshRenderable>> h_Renderables_Bunny0;
//std::vector<Handle<MeshRenderable>> h_Renderables_Bunny1;
//std::vector<Handle<MeshRenderable>> h_Renderables_Bunny2;
//std::vector<Handle<MeshRenderable>> h_Renderables_Bunny3;

//struct GameObject
//{
    //std::vector<Handle<MeshRenderable>> h_Renderables;

    //void SetTransform(m4f transformMatrix)
    //{
        //for(i32 i = 0; i < h_Renderables.size(); i++)
        //{
            //Renderer_GetMeshRenderable(h_Renderables[i]).u_Model = transformMatrix;
        //}
    //}
//};

//void SpawnRandomBunny(Handle<ShaderPipeline> h_Shader)
//{
    //GameObject bunny;
    //bunny.h_Renderables = Renderer_CreateRenderablesFromModel(MODELS_PATH"bunny/bunny.obj", h_Shader);
    //v3f pos =
    //{
        //RandomRange(-2.f, 2.f),
        //RandomRange(-2.f, 2.f),
        //RandomRange(-2.f, 2.f),
    //};
    //f32 angle = RandomRange(0.f, 360.f);
    //v3f axis =
    //{
        //RandomRange(0.f, 1.f),
        //RandomRange(0.f, 1.f),
        //RandomRange(0.f, 1.f),
    //};
    //axis = Normalize(axis);
    //f32 scale = RandomRange(0.2f, 1.5f);
    //bunny.SetTransform(TranslationMatrix(pos) * RotationMatrix(angle, axis) * ScaleMatrix({scale, scale, scale}));
//}

//GameObject sponza;
//std::vector<GameObject> bunnies;

std::vector<AudioWaveVertex> audioWaveVertices;
std::vector<u32> audioWaveIndices;
v3f audioWaveCenter = {};
std::string srcVS_AudioWave;
std::string srcPS_AudioWave;
Handle<AudioWaveRenderable> h_AudioWaveRenderable;

MemArena memArena_Audio;

// Frequency version (would be more interesting if I was better at DSP)
#define AUDIO_BUFFER_SIZE 1024
#define MAGNITUDE_FACTOR 1e8

f64 lastAudioSamples[AUDIO_BUFFER_SIZE];
f64 fftOutReal[AUDIO_BUFFER_SIZE];
f64 fftOutImag[AUDIO_BUFFER_SIZE];

f32 sampleFrequencies[AUDIO_BUFFER_SIZE/2];

// Pitch version
f32 highestPitch = 0.f;

void AudioDataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount)
{
    ma_decoder* decoder = (ma_decoder*)device->pUserData;
    ASSERT(decoder);
    ma_decoder_read_pcm_frames(decoder, output, frameCount, NULL);

    // // Copying new samples to audio buffer
    // i32 n = AUDIO_BUFFER_SIZE - frameCount;
    // f64 newSamples[AUDIO_BUFFER_SIZE];
    // for(i32 i = 0; i <= AUDIO_BUFFER_SIZE; i++)
    // {
    //     newSamples[i] = lastAudioSamples[(i + n) % AUDIO_BUFFER_SIZE];
    // }
    // for(i32 i = 0; i < frameCount; i++)
    // {
    //     i32 sample = AUDIO_BUFFER_SIZE - frameCount + i;
    //     newSamples[i] = (f64)*((f64*)output + i);
    // }
    // memcpy(lastAudioSamples, newSamples, sizeof(lastAudioSamples));

    // fft(lastAudioSamples, nullptr, AUDIO_BUFFER_SIZE, fftOutReal, fftOutImag);
    // f32 outputMagnitudes[AUDIO_BUFFER_SIZE/2];
    // for(i32 i = 0; i < AUDIO_BUFFER_SIZE/2; i++)
    // {
    //     outputMagnitudes[i] = sqrt(fftOutReal[i] * fftOutReal[i] + fftOutImag[i] * fftOutImag[i]);
    //     outputMagnitudes[i] *= MAGNITUDE_FACTOR;
    // }

    // //for(i32 i = 0; i < AUDIO_BUFFER_SIZE/2; i++)
    // //{
    //     //f32 v = (outputMagnitudes[i] - avgMagnitude)/(maxMagnitude - avgMagnitude);
    //     //outputMagnitudes[i] = v;
    //     ////outputMagnitudes[i] = (outputMagnitudes[i] - avgMagnitude) * (f32)MAGNITUDE_FACTOR;
    // //}

    // memcpy(sampleFrequencies, outputMagnitudes, sizeof(sampleFrequencies));

    // Pitch version
    f32 maxSample = 0.f;
    for(i32 i = 0; i < frameCount; i++)
    {
        f32 sample = (f32)*((f32*)output + i);
        maxSample = MAX(sample, maxSample);
    }
    highestPitch = maxSample;
}

ma_device audioDevice;
ma_device_config audioDeviceConfig;
ma_decoder audioDecoder;

Timer appTimer;

void App_Init(u32 windowWidth, u32 windowHeight, const char* appTitle)
{
    // Initializing common engine systems
    Time_Init();

    // Initializing renderer
    Renderer_Init(windowWidth, windowHeight, appTitle, &appWindow);
    Camera mainCamera = {};
    mainCamera.Move({-6, 20, 15});
    Renderer_SetCamera(mainCamera);

    // Creating app resources and renderables
    //srcVS_Default = ReadFile_Str(SHADER_PATH"default_vertex.vs");
    //srcPS_Default = ReadFile_Str(SHADER_PATH"default_pixel.ps");
    //Handle<Shader> h_VS_Default = Renderer_CreateShader(srcVS_Default, SHADER_TYPE_VERTEX);
    //Handle<Shader> h_PS_Default = Renderer_CreateShader(srcPS_Default, SHADER_TYPE_PIXEL);
    //Handle<ShaderPipeline> h_Shader_Default = Renderer_CreateShaderPipeline(h_VS_Default, h_PS_Default);
    srcVS_AudioWave = ReadFile_Str(SHADER_PATH"audiowave.vs");
    srcPS_AudioWave = ReadFile_Str(SHADER_PATH"audiowave.ps");
    Handle<Shader> h_VS_AudioWave = Renderer_CreateShader(srcVS_AudioWave, SHADER_TYPE_VERTEX);
    Handle<Shader> h_PS_AudioWave = Renderer_CreateShader(srcPS_AudioWave, SHADER_TYPE_PIXEL);
    Handle<ShaderPipeline> h_Shader_AudioWave = Renderer_CreateShaderPipeline(h_VS_AudioWave, h_PS_AudioWave);

    //sponza.h_Renderables = Renderer_CreateRenderablesFromModel(MODELS_PATH"sponza/sponza.obj", h_Shader_Default);
    //m4f sponzaWorld = RotationMatrix(TO_RAD(90.f), {0.f, 1.f, 0.f}) * ScaleMatrix({0.01f, 0.01f, 0.01f});
    //sponza.SetTransform(sponzaWorld);

    //for(i32 i = 0; i < 20; i++)
    //{
        //SpawnRandomBunny(h_Shader_Default);
    //}


    // App settings
    Input_LockMouse(true);
    Input_ShowMouse(false);
    
    // Audio Memory
    MemArena_Init(&memArena_Audio, MB(100));

    // Reading audio file
    std::string_view audioPath(RESOURCE_PATH"audio/mata_ashita.mp3");
    u64 fSize = GetFileSize(audioPath);
    u8* fBuffer = (u8*)MemArena_Alloc(&memArena_Audio, fSize);
    ReadFile(audioPath, fBuffer);

    memset(lastAudioSamples, 0, sizeof(lastAudioSamples));

    ma_result ret;
    ma_decoder_config decoderConfig = {};
    decoderConfig.format = ma_format_f32;
    decoderConfig.sampleRate = 44100;
    decoderConfig.channels = 2;
    ret = ma_decoder_init_memory(fBuffer, fSize, &decoderConfig, &audioDecoder);
    ASSERT(ret == MA_SUCCESS);

    audioDeviceConfig = ma_device_config_init(ma_device_type_playback);
    audioDeviceConfig.playback.format = audioDecoder.outputFormat;
    audioDeviceConfig.playback.channels = audioDecoder.outputChannels;
    audioDeviceConfig.sampleRate = audioDecoder.outputSampleRate;
    audioDeviceConfig.playback.format = audioDecoder.outputFormat;
    audioDeviceConfig.dataCallback = AudioDataCallback;
    audioDeviceConfig.pUserData = &audioDecoder;
    ret = ma_device_init(NULL, &audioDeviceConfig, &audioDevice);
    ASSERT(ret == MA_SUCCESS);

    ret = ma_device_start(&audioDevice);
    ASSERT(ret == MA_SUCCESS);

    rendererData.audioWaveRenderable.h_Shader = h_Shader_AudioWave;

    // Initializing vertices
    const f32 meshSide = 30.f;
    v3f meshCenter = {meshSide/2.f, 0.f, meshSide/2.f};
    f32 maxDist = Len(meshCenter - v3f{0.f,0.f,0.f});
    audioWaveVertices.reserve(512 * 512);
    audioWaveIndices.reserve((512 * 512 / 2) * 3);  // Is this correct?
    for(i32 i = 0; i < 512; i++)
    {
        for(i32 j = 0; j < 512; j++)
        {
            v3f pos = {((f32)i / (f32)512) * meshSide, 0.f, ((f32)j / (f32)512) * meshSide};
            f32 dist = Len(pos - meshCenter) / maxDist;
            AudioWaveVertex v = {pos, dist};
            audioWaveVertices.push_back(v);
        }
    }

    for(i32 i = 0; i < 512 - 1; i++)
    {
        for(i32 j = 0; j < 512 - 1; j++)
        {
            u32 quadIndices[] =
            {
                (u32)(i * 512 + j),
                (u32)(i * 512 + (j + 1)),
                (u32)((i + 1) * 512 + (j + 1)),
                (u32)((i + 1) * 512 + j),
            };
            audioWaveIndices.push_back(quadIndices[0]);
            audioWaveIndices.push_back(quadIndices[1]);
            audioWaveIndices.push_back(quadIndices[2]);
            audioWaveIndices.push_back(quadIndices[0]);
            audioWaveIndices.push_back(quadIndices[2]);
            audioWaveIndices.push_back(quadIndices[3]);
        }
    }

    Handle<Buffer> audioWaveVB = Renderer_CreateBuffer((u8*)audioWaveVertices.data(), audioWaveVertices.size(), sizeof(AudioWaveVertex), BUFFER_TYPE_VERTEX);
    Handle<Buffer> audioWaveIB = Renderer_CreateBuffer((u8*)audioWaveIndices.data(), audioWaveIndices.size(), sizeof(u32), BUFFER_TYPE_INDEX);
    rendererData.audioWaveRenderable.h_Mesh = Renderer_CreateMesh(audioWaveVB, audioWaveIB, true);

    StartTimer(&appTimer);
}

void App_Update()
{
    // Update common systems
    Input_UpdateState();

    // Camera controls
    static bool controlCamInput = true;
    if(controlCamInput)
    {
        Camera& mainCamera = Renderer_GetCamera();

        const f32 cameraRotationSpeed = 0.01f;
        v2f mouseDelta = Input_GetMouseDelta();
        mainCamera.RotateYaw(cameraRotationSpeed * -mouseDelta.y);
        mainCamera.RotatePitch(cameraRotationSpeed * -mouseDelta.x);

        const f32 cameraSpeed = 0.01f;
        v2f cameraMoveAmount = {};
        if      (Input_IsKeyDown(KEY_W)) cameraMoveAmount.y =  1.f * cameraSpeed;
        else if (Input_IsKeyDown(KEY_S)) cameraMoveAmount.y = -1.f * cameraSpeed;
        if      (Input_IsKeyDown(KEY_A)) cameraMoveAmount.x = -1.f * cameraSpeed;
        else if (Input_IsKeyDown(KEY_D)) cameraMoveAmount.x =  1.f * cameraSpeed;

        mainCamera.Move(mainCamera.position
                + mainCamera.front * cameraMoveAmount.y
                + mainCamera.right * cameraMoveAmount.x);
    }
    
    // App controls
    if(Input_IsKeyJustDown(KEY_ESCAPE))
    {
        appWindow.shouldClose = true;
    }
    if(Input_IsKeyJustDown(KEY_P))
    {
        Input_ToggleLockMouse();
        Input_ToggleShowMouse();
    }
    if(Input_IsKeyJustDown(KEY_O))
    {
        controlCamInput = !controlCamInput;
    }

    // Wave propagation
    //const f32 waveFreq = 2.f;   // 2Hz
    //EndTimer(&appTimer);
    //f32 t = GetTime_S(appTimer);
    //rendererData.audioWaveRenderable.PropagateWave(sinf(2.f * PI * waveFreq * t));
    //rendererData.audioWaveRenderable.SetWaveData(lastAudioSamples);
    //rendererData.audioWaveRenderable.SetWaveData(sampleFrequencies);
    rendererData.audioWaveRenderable.PropagateWave(highestPitch);
}

void App_Render()
{
    Renderer_RenderFrame();
}

void App_Destroy()
{
    // Destroy app window
    Window_Destroy(appWindow);
}

} // namespace Ty
