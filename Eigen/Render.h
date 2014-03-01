#pragma once

namespace Eigen
{
    class Display;
    class Stage;


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderBatch
    //


    struct RenderBatch
    {
        //ShaderId      shader;
        //GeometryId    geometry;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderPort
    //

    struct RenderPort
    {
        const char*     name;
        int             index;

    protected:

        RenderPort()    {}
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderContext
    //

    class RenderContext
    {
    public:

        void            CommitBatch(const RenderPort* port, const RenderBatch& batch, float sortDepth);

        struct          Details;

    protected:

        RenderContext() {}
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderSystem
    //

    class RenderSystem
    {
    public:

        const RenderPort*   GetPort(const char* name);
        RenderContext*      BeginContext();
        void                EndContext(RenderContext* context, Stage* stages, int stageCount);

        class               Details;
        class               Core;

    protected:

        RenderSystem()      {}
    };
}
