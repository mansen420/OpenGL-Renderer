namespace renderer
{
    namespace preprocessor
    {
        //delete[ ]'ing the processed source holder is the caller's responsibility!
        //FRIENDLY REMINDER : never mutate a pointer you are going to delete later : )
        bool process_shader(const char* const source, char* &processed_source_holder);
        //
        void write_unrolled_shaders();
    }
}