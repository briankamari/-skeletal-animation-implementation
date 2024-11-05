#include "collada.h"

inline _mat4 generatematrix(const char* string){
    _float_array matrix_array = _stringtofloats( _string(string) );
    _mat4 result;
    float * matrix = (float*)&(result);
    for(int i=0;i<16;i++){ matrix[i]=matrix_array[i]; }
    return rt_transpose(result);
}

inline _matrix_array floatstomatrix(const uint32_t& count,const _float_array& floats){
    _matrix_array result;
    for(uint32_t i=0;i<count;i++){
        uint32_t pos = i*16;
        _mat4 transform;
        float * matrix = (float*)(&transform);
        for(uint32_t ii=0;ii<16;ii++){ matrix[ii] = floats[pos+ii]; }
        result.pushback( rt_transpose(transform),true );
    }
    return result;
}

int collada::findsource(const char* id){
    for(uint32_t i=0;i<m_sources.m_count; i++){
        if( app_scm(id,m_sources[i].m_id.m_data) ) { return i; }
    }
    return -1;
}

bool readsource(XMLElement * element,collada_source * source){

    if(!element || !source ){ app_throw("source"); }

    source->m_id = element->Attribute("id");

    rt_firstchild(element);
    while(element){

        bool namearray  = app_scm("Name_array" ,element->Name());
        bool floatarray = app_scm("float_array",element->Name());
        if( namearray || floatarray ){
            source->m_string_array = _stringsplit(element->GetText());
            if(floatarray){ source->m_float_array = _stringtofloats(source->m_string_array); }
        }
        if( app_scm("technique_common",element->Name() ) ){
            source->m_accessor_source = (element->FirstChildElement()->Attribute("source") + 1);
            source->m_accessor_count  = atoi(element->FirstChildElement()->Attribute("count") );
            source->m_accessor_stride = atoi(element->FirstChildElement()->Attribute("stride") );
        }
        rt_nextsibling(element);
    }
    return true;
}

bool collada::loadfile(const char* path){

    tinyxml2::XMLDocument doc;
    if(doc.LoadFile(path)  != XML_SUCCESS){ app_throw("xml"); }

    XMLElement * root = doc.FirstChildElement()->FirstChildElement();
    if(!root) { app_throw("xml"); }
    while(root){
        if( app_scm("library_geometries",root->Name() )    ) { if(!readgeometries(root)){return false;} }
        if( app_scm("library_animations",root->Name() )    ) { readanimations(root);}
        if( app_scm("library_controllers",root->Name() )   ) { readcontrollers(root); }
        if( app_scm("library_visual_scenes",root->Name() ) ) { readvisualscenes(root); }
        root = root->NextSiblingElement();
    }
    return true;
}

bool collada::readgeometries(XMLElement * element){

    XMLElement * geometry_element = element->FirstChildElement();
    if(!geometry_element || geometry_element->NextSiblingElement() ) { app_throw("xml"); }

    _string id = geometry_element->Attribute("id");
    _string name = geometry_element->Attribute("name");

    rt_firstchild(geometry_element);
    rt_firstchild(geometry_element);
    while(geometry_element){

        if( app_scm("source",geometry_element->Name() ) ){
            collada_source source;
            if(!readsource(geometry_element,&source)){ return false; }
            m_sources.pushback(source,true);
        }
        if( app_scm("vertices",geometry_element->Name() ) ){
            m_vertexsource =((geometry_element->FirstChildElement()->Attribute("source"))+1);
            if(m_vertexsource.m_count==0){ app_throw("vertex source");}
        }
        if( app_scm("polylist",geometry_element->Name() ) ){

            collada_polylist  polylist;
            polylist.m_material = geometry_element->Attribute("material");
            polylist.m_count    = atoi(geometry_element->Attribute("count"));
            XMLElement * polylist_element = geometry_element->FirstChildElement();
            while(polylist_element){
                if(app_scm("input",polylist_element->Name()) ){
                    collada_input input;
                    input.m_semantic = polylist_element->Attribute("semantic");
                    input.m_source   = (polylist_element->Attribute("source")+1);

                    const char* offset = polylist_element->Attribute("offset");
                    if(offset){ input.m_offset = uint16_t( atoi(offset) );}
                    const char* set    = polylist_element->Attribute("set");
                    if(set)   { input.m_set    = uint16_t( atoi(set) );   }

                    polylist.m_inputs.pushback(input,true);
                }
                if(app_scm("vcount",polylist_element->Name()) ){
                    polylist.m_vcount = _stringtoints(_string(polylist_element->GetText()));
                }
                if(app_scm("p",polylist_element->Name()) ){
                    polylist.m_p     = _stringtoints(_string(polylist_element->GetText()));
                }
                rt_nextsibling(polylist_element);
            }
            m_polylists.pushback(polylist,true);
        }
        rt_nextsibling(geometry_element);
    }
    return true;
}

_string gettarget(const char* string){
    uint32_t length = strlen(string);
    _string result;
    if(length){
        for(uint32_t i =0;i<length; i++){
            if(string[i] == '/'){ return result; }
            else{ result.pushback(string[i],true); }
        }
    }
    return result;
}

void collada::readanimations(XMLElement * element){
    XMLElement * animation_element = element->FirstChildElement();
    if(!animation_element  ) { return; }

    static bool readinputs = false;
    while(animation_element){

        if(app_scm("animation",animation_element->Name()) ){

            rt_animations animation;
            XMLElement * animations_element = animation_element->FirstChildElement();
            while(animations_element){
                if(app_scm("source",animations_element->Name()) ){
                    collada_source source;
                    if(!readsource(animations_element,&source)){
                        app_error("source");
                        return;
                    }
                    m_sources.pushback(source,true);
                }
                if(app_scm("sampler",animations_element->Name()) ){
                    XMLElement * sampler_element = animations_element->FirstChildElement();
                    while(sampler_element){
                        if(app_scm("OUTPUT", sampler_element->Attribute("semantic") ) ){
                            animation.m_output_source = (sampler_element->Attribute("source")+1);
                        }
                        if(app_scm("INPUT", sampler_element->Attribute("semantic") ) ){
                            if(!readinputs){
                                m_keyframe_source = (sampler_element->Attribute("source")+1);
                                readinputs = true;
                            }
                        }
                        rt_nextsibling(sampler_element);
                    }
                }
                if(app_scm("channel",animations_element->Name()) ){
                    animation.m_target = gettarget(animations_element->Attribute("target"));
                }
                rt_nextsibling(animations_element);
            }
            m_animations.pushback(animation);
        }
        rt_nextsibling(animation_element);
    }
}

void collada::readcontrollers(XMLElement * element){

    XMLElement * controller_element = element->FirstChildElement();
    if(!controller_element  ) { return; }

    m_controller.m_id   = controller_element->Attribute("id");
    m_controller.m_name = controller_element->Attribute("name");

    controller_element = controller_element->FirstChildElement()->FirstChildElement();
    if(!controller_element  ) { app_error("xml"); }

    while(controller_element){

        if(app_scm("bind_shape_matrix",controller_element->Name())){
            m_controller.m_bind_shape_matrix = generatematrix(controller_element->GetText());
        }
        if(app_scm("source",controller_element->Name())){
            collada_source source;
            if(!readsource(controller_element,&source)){
                app_error("source");
                return;
            }
            m_sources.pushback(source,true);
        }
        if(app_scm("joints",controller_element->Name())){
            XMLElement * joints = controller_element->FirstChildElement();
            while(joints){
                if(app_scm("JOINT",joints->Attribute("semantic"))){
                    m_controller.m_joint_source = (joints->Attribute("source")+1);
                }
                if(app_scm("INV_BIND_MATRIX",joints->Attribute("semantic"))){
                    m_controller.m_inverse_bind_source=(joints->Attribute("source")+1);
                }
                rt_nextsibling(joints);
            }
        }
        if(app_scm("vertex_weights",controller_element->Name())){
            XMLElement * vertex_weights = controller_element->FirstChildElement();
            while(vertex_weights){
                if( app_scm("input",vertex_weights->Name()) ){
                    if(app_scm("WEIGHT",vertex_weights->Attribute("semantic"))){
                        m_controller.m_joint_weight_source = (vertex_weights->Attribute("source")+1);
                    }
                }
                if( app_scm("vcount",vertex_weights->Name()) ){
                    m_controller.m_vcount = _stringtoints(_string( vertex_weights->GetText()) );
                }
                if( app_scm("v",vertex_weights->Name()) ){
                    m_controller.m_v = _stringtoints(_string( vertex_weights->GetText()) );
                }
                rt_nextsibling(vertex_weights);
            }
        }
        rt_nextsibling(controller_element);
    }
}

XMLElement * findroot(XMLElement * element,const _string& id = ""){

    if(!element){ return NULL;}
    if(id.m_count==0){
        if(app_scm("instance_controller",element->Name())){
            XMLElement * skeleton = element->FirstChildElement();
            if(skeleton && app_scm(skeleton->Name(),"skeleton") ){ return skeleton; }
        }
    }else{
        if(app_scm("node",element->Name()) && app_scm(id.m_data,element->Attribute("id")) ){ return element; }
    }
    XMLElement * nodes = element->FirstChildElement();

    while(nodes){
        XMLElement * node = findroot(nodes,id);
        if(node){ return node; }
        rt_nextsibling(nodes);
    }
    return NULL;
}

bool readnodes(XMLElement * element,rt_node* root){

    if(!element || !root || (!app_scm(element->Name(),"node")) ){ app_throw("readnodes");}

    XMLElement * matrix = element->FirstChildElement();
    if( !matrix || (!app_scm(matrix->Name(),"matrix")) ){ app_throw("readnodes"); }

    _mat4 transform = generatematrix(matrix->GetText());

    if(!element->Attribute("id")){ app_throw("readnodes"); }
    rt_node* node = root->addnode(element->Attribute("id"),transform);

    XMLElement * nodes = matrix->NextSiblingElement();
    while(nodes){
        if(!readnodes(nodes,node)){ app_throw("readnodes"); }
        rt_nextsibling(nodes);
    }
    return true;
}

void collada::readvisualscenes(XMLElement * element){

    XMLElement * visualscene_element = element->FirstChildElement();
    if( !visualscene_element ) { return; }

    XMLElement * root = findroot(visualscene_element);
    if(root){
        XMLElement * node = findroot(visualscene_element,(root->GetText()+1));
        if(node){
            m_root_node = new rt_node();
            m_root_node->m_id = "RT_ROOT_NODE";
            if(!readnodes(node,m_root_node)){ app_error("readnodes"); }
        }
    }
}

bool collada::generatetransforms(rt_node*node,const _mat4&parent,int32_t animation_index){

    if(!node){ app_throw("generatetransforms"); }

    int32_t boneindex = -1;
    for(uint32_t i=0;i<m_bone_count;i++){
        if(app_scm(m_bonenames[i].m_data,node->m_id.m_data)){ boneindex = i; break; }
    }
    if(boneindex == -1){ app_throw("boneindex"); }

    _mat4 T;
    if(animation_index>=0){
        T=m_keyframes[animation_index][boneindex]*parent;
        m_keyframes[animation_index][boneindex] = m_controller.m_bind_shape_matrix*m_inv_transforms[boneindex]*T;
    }else{
        T=node->m_transform*parent;
        m_transforms[boneindex] = m_controller.m_bind_shape_matrix*m_inv_transforms[boneindex]*T;
    }
    for(uint32_t i=0;i<node->m_count;i++){
        if(!generatetransforms(node->m_children[i],T,animation_index)){ return false; }
    }
    return true;
}

bool collada::genaratefile(const char* path){

    if(!path){ app_throw("path"); }

    _vec3 * vertices     = NULL;
    _vec3 * normals      = NULL;
    _vec2 * textures     = NULL;
    int vertex_source_id   = -1;
    int normal_source_id   = -1;
    int texture_source_id  = -1;

    for(uint32_t i=0;i<m_polylists[0].m_inputs.m_count; i++ ){
        collada_input & input =m_polylists[0].m_inputs[i];
        if( app_scm("VERTEX",input.m_semantic.m_data) ){
            vertex_source_id = findsource(m_vertexsource.m_data);
            if( vertex_source_id <0 ){ app_throw("genaratefile"); }
            vertices = new _vec3[ m_sources[vertex_source_id].m_accessor_count];
            for(uint32_t v_i=0,f_i=0, ii=0;ii<m_sources[vertex_source_id].m_accessor_count;ii++){
                vertices[v_i].x = m_sources[vertex_source_id].m_float_array[f_i++];
                vertices[v_i].y = m_sources[vertex_source_id].m_float_array[f_i++];
                vertices[v_i].z = m_sources[vertex_source_id].m_float_array[f_i++];
                v_i++;
            }
        }
        else if( app_scm("NORMAL",input.m_semantic.m_data) ){
            normal_source_id = findsource(input.m_source.m_data);
            if( normal_source_id < 0 ){ app_throw("genaratefile"); }
            normals = new _vec3[ m_sources[normal_source_id].m_accessor_count];
            for(uint32_t n_i=0,f_i=0, ii=0;ii<m_sources[normal_source_id].m_accessor_count;ii++){
                normals[n_i].x = m_sources[normal_source_id].m_float_array[f_i++];
                normals[n_i].y = m_sources[normal_source_id].m_float_array[f_i++];
                normals[n_i].z = m_sources[normal_source_id].m_float_array[f_i++];
                n_i++;
            }
        }
        else if( app_scm("TEXCOORD",input.m_semantic.m_data) ){
            texture_source_id = findsource(input.m_source.m_data);
            if( texture_source_id < 0 ){  app_throw("genaratefile"); }
            textures = new _vec2[ m_sources[texture_source_id].m_accessor_count];
            for(uint32_t t_i=0,f_i=0, ii=0;ii<m_sources[texture_source_id].m_accessor_count;ii++){
                textures[t_i].x = m_sources[texture_source_id].m_float_array[f_i++];
                textures[t_i].y = m_sources[texture_source_id].m_float_array[f_i++];
                t_i++;
            }
        }else { app_throw("genaratefile"); }
    }
    if( !vertices ){ app_throw("no vertices found"); }
    if( !normals  ){ app_throw("no normals found"); }
    if( !textures ){ app_throw("no textures found"); }
    if( (vertex_source_id==-1) || (normal_source_id==-1) || (texture_source_id==-1) ){ app_throw("sources"); }

    _vec2 *joints = NULL;
    _vec2 *weights = NULL;
    if(m_root_node){

        int joint_src = findsource(m_controller.m_joint_source.m_data);
        int inv_joint_src = findsource(m_controller.m_inverse_bind_source.m_data);
        int joint_weight_src = findsource(m_controller.m_joint_weight_source.m_data);
        if( (joint_src==-1) || (inv_joint_src==-1) || (joint_weight_src==-1) ){ app_throw("genaratefile"); }

        m_bone_count = uint16_t(m_sources[joint_src].m_accessor_count);

        uint32_t vertex_count = m_controller.m_vcount.m_count;
        joints = new _vec2[vertex_count];
        weights = new _vec2[vertex_count];

        int32_t index = 0;
        collada_source & weightsource = m_sources[joint_weight_src];
        for(uint32_t vcount_i =0;vcount_i<vertex_count;vcount_i++){
            for(int32_t v_i =0;v_i<m_controller.m_vcount[vcount_i];v_i++){
                joints [vcount_i][v_i] = float(m_controller.m_v[index++]);
                weights[vcount_i][v_i] = weightsource.m_float_array[ m_controller.m_v[index++] ];
            }
        }
        m_transforms.allocate(m_bone_count);
        m_bonenames = m_sources[joint_src].m_string_array;
        m_inv_transforms = floatstomatrix(
                    m_sources[inv_joint_src].m_accessor_count,
                    m_sources[inv_joint_src].m_float_array);

        generatetransforms(m_root_node->m_children[0],_mat4(),-1);

        if(m_keyframe_source.m_count){
            int keyframe_src = findsource(m_keyframe_source.m_data);
            if(keyframe_src>=0){
                m_keyframe_inputs = m_sources[keyframe_src].m_float_array;
                m_keyframe_count = uint16_t(m_keyframe_inputs.m_count);

                _transform_array frames;
                for(uint32_t i=0;i<m_animations.m_count;i++){

                    int frame_src = findsource( m_animations[i].m_output_source.m_data );
                    if( frame_src == -1){ app_throw("findsource"); }
                    _matrix_array frame=floatstomatrix(
                                m_sources[frame_src].m_accessor_count,
                                m_sources[frame_src].m_float_array);
                    frames.pushback(frame);
                }
                m_keyframes.alloc(m_keyframe_count);
                for(uint32_t i=0;i<m_keyframe_count;i++){ m_keyframes[i].allocate(m_bone_count); }

                for(uint32_t i=0;i<m_bone_count;i++){
                    for(uint32_t ii=0;ii<m_keyframe_count;ii++){ m_keyframes[ii][i] = frames[i][ii]; }
                }
            }
        }
        for(uint32_t i =0;i<m_keyframe_count;i++){
            generatetransforms(m_root_node->m_children[0],_mat4(),i);
        }
    }
    _submeshes submeshes;
    for(uint32_t i=0;i<m_polylists.m_count; i++ ){
        collada_polylist & polylist = m_polylists[i];

        _submesh  submesh;
        for(int32_t index =0 , count_i=0;count_i<polylist.m_count;count_i++){

            for(int32_t vcount_i=0; vcount_i <polylist.m_vcount[count_i];vcount_i++){
                _vertex vertex_;
                for(uint32_t input_i=0;input_i<polylist.m_inputs.m_count; input_i++ ){

                    if( app_scm("VERTEX",polylist.m_inputs[input_i].m_semantic.m_data) ){
                        vertex_.m_vertex = vertices[ polylist.m_p[index] ];
                        if(m_root_node){
                            vertex_.m_bone_indexes = joints[ polylist.m_p[index] ];
                            vertex_.m_bone_weights = weights[ polylist.m_p[index] ];
                        }
                        index++;
                    }
                    if( app_scm("NORMAL",polylist.m_inputs[input_i].m_semantic.m_data) ){
                        vertex_.m_normal = normals[ polylist.m_p[index++] ];
                    }
                    if( app_scm("TEXCOORD",polylist.m_inputs[input_i].m_semantic.m_data) ){
                        vertex_.m_uv = textures[  polylist.m_p[index++] ];
                    }
                }
                submesh.m_indices.pushback(submesh.m_vertices.m_count,true);
                submesh.m_vertices.pushback(vertex_,true);
            }
        }
        submeshes.pushback(submesh,true);
    }

    FILE * file;
    fopen_s(&file,path,"wb");
    if(!file){ app_throw("file"); }

    const char * header = "_mesh_";
    if( fwrite(header,1,6,file) != 6 ){ app_throw("fwrite"); }

    uint16_t submesh_count = uint16_t(submeshes.m_count);
    if( fwrite( (&submesh_count) ,2,1,file) != 1 ){ app_throw("fwrite"); }

    for( uint32_t i=0;i<submeshes.m_count;i++ ){

        uint32_t index_count = submeshes[i].m_indices.m_count;
        if( fwrite( (&index_count) ,4,1,file) != 1 ){ app_throw("fwrite"); }
        for(uint32_t ii=0;ii<index_count;ii++){
            if( fwrite( (&(submeshes[i].m_indices[ii])) ,4,1,file) != 1 ){ app_throw("fwrite"); }
        }
        uint32_t vertex_count = submeshes[i].m_vertices.m_count;
        printf("submesh: %i vertex_count: %i \n",i,vertex_count);
        if( fwrite( (&vertex_count) ,4,1,file) != 1 ){ app_throw("fwrite"); }
        for(uint32_t ii=0;ii<vertex_count;ii++){
            if( fwrite( (&(submeshes[i].m_vertices[ii])) ,sizeof(_vertex),1,file) != 1 ){ app_throw("fwrite"); }
        }
    }
    if(m_bone_count){
        printf("bone_count: %i \n",m_bone_count);
        if( fwrite( (&m_bone_count) ,2,1,file) != 1 ){ app_throw("fwrite"); }
        if( fwrite( (&(m_transforms[0])) ,sizeof(_mat4),m_bone_count,file) != m_bone_count ){ app_throw("fwrite"); }

        if( fwrite( (&m_keyframe_count) ,2,1,file) != 1 ){ app_throw("fwrite"); }
        if(m_keyframe_count){
            printf("keyframes: %i \n",m_keyframe_count);
            for(uint32_t ii=0;ii<m_keyframe_count;ii++){
                if( fwrite( (&(m_keyframes[ii][0])) ,sizeof(_mat4),m_bone_count,file) != m_bone_count ){ app_throw("fwrite"); }
            }
        }
        rt_node::delete_nodes(m_root_node);
    }
    return true;
}
