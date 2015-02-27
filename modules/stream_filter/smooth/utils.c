/*****************************************************************************
 * utils.c: misc. stuff
 *****************************************************************************
 * Copyright (C) 1996-2012 VLC authors and VideoLAN
 * $Id: 5c180dc29e406b5b6a44a65c2424ff0d12fd075d $
 *
 * Author: Frédéric Yhuel <fyhuel _AT_ viotech _DOT_ net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *****************************************************************************/

#include "stdafx.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_es.h>
#include <vlc_block.h>
#include <assert.h>

#include "smooth.h"

static int hex_digit( const char c )
{
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= '0' && c<= '9')
        return c - '0';
    else
        return -1;
}

uint8_t *decode_string_hex_to_binary( const char *psz_src )
{
    int i = 0, j = 0, first_digit, second_digit;
    int i_len = strlen( psz_src );
    assert( i_len % 2 == 0 );
    uint8_t *p_data = (uint8_t *)malloc( i_len / 2 );			// sunqueen modify

    if( !p_data )
        return NULL;

    while( i < i_len )
    {
        first_digit = hex_digit( psz_src[i++] );
        second_digit = hex_digit( psz_src[i++] );
        assert( first_digit >= 0 && second_digit >= 0 );
        p_data[j++] = ( first_digit << 4 ) | second_digit;
    }

    return p_data;
}

quality_level_t * ql_New( void )
{
    quality_level_t *ql = (quality_level_t *)calloc( 1, sizeof( quality_level_t ) );			// sunqueen modify
    if( unlikely( !ql ) ) return NULL;

    ql->Index = -1;
    return ql;
}

void ql_Free( quality_level_t *qlevel )
{
    free( qlevel->CodecPrivateData );
    free( qlevel );
    qlevel = NULL;
}

chunk_t *chunk_New( sms_stream_t* sms, uint64_t duration,\
        uint64_t start_time )
{
    chunk_t *chunk = (chunk_t *)calloc( 1, sizeof( chunk_t ) );			// sunqueen modify
    if( unlikely( chunk == NULL ) )
        return NULL;

    chunk->duration = duration;
    chunk->start_time = start_time;
    chunk->type = UNKNOWN_ES;
    chunk->sequence = vlc_array_count( sms->chunks );
    vlc_array_append( sms->chunks, chunk );
    return chunk;
}

void chunk_Free( chunk_t *chunk )
{
    if( chunk->data )
        FREENULL( chunk->data );
    FREENULL( chunk );
}

sms_stream_t * sms_New( void )
{
    sms_stream_t *sms = (sms_stream_t *)calloc( 1, sizeof( sms_stream_t ) );			// sunqueen modify
    if( unlikely( !sms ) ) return NULL;

    sms->qlevels = vlc_array_new();
    sms->chunks = vlc_array_new();
    sms->type = UNKNOWN_ES;
    return sms;
}

void sms_Free( sms_stream_t *sms )
{
    if( sms->qlevels )
    {
        for( int n = 0; n < vlc_array_count( sms->qlevels ); n++ )
        {
            quality_level_t *qlevel = (quality_level_t *)vlc_array_item_at_index( sms->qlevels, n );			// sunqueen modify
            if( qlevel ) ql_Free( qlevel );
        }
        vlc_array_destroy( sms->qlevels );
    }

    if( sms->chunks )
    {
        for( int n = 0; n < vlc_array_count( sms->chunks ); n++ )
        {
            chunk_t *chunk = (chunk_t *)vlc_array_item_at_index( sms->chunks, n );			// sunqueen modify
            if( chunk) chunk_Free( chunk );
        }
        vlc_array_destroy( sms->chunks );
    }

    free( sms->name );
    free( sms->url_template );
    free( sms );
    sms = NULL;
}

quality_level_t *get_qlevel( sms_stream_t *sms, const unsigned qid )
{
    quality_level_t *qlevel = NULL;
    for( unsigned i = 0; i < sms->qlevel_nb; i++ )
    {
        qlevel = (quality_level_t *)vlc_array_item_at_index( sms->qlevels, i );			// sunqueen modify
        if( qlevel->id == qid )
            return qlevel;
    }
    return NULL;
}

sms_queue_t *sms_queue_init( const int length )
{
    sms_queue_t *ret = (sms_queue_t *)malloc( sizeof( sms_queue_t ) );			// sunqueen modify
    if( unlikely( !ret ) )
        return NULL;
    ret->length = length;
    ret->first = NULL;
    return ret;
}

void sms_queue_free( sms_queue_t* queue )
{
    item_t *item = queue->first, *next = NULL;
    while( item )
    {
        next = item->next;
        FREENULL( item );
        item = next;
    }
    FREENULL( queue );
}

int sms_queue_put( sms_queue_t *queue, const uint64_t value )
{
    /* Remove the last (and oldest) item */
    item_t *item, *prev = NULL;
    int count = 0;
    for( item = queue->first; item != NULL; item = item->next )
    {
        count++;
        if( count == queue->length )
        {
            FREENULL( item );
            if( prev ) prev->next = NULL;
            break;
        }
        else
            prev = item;
    }

    /* Now insert the new item */
    item_t *_new = (item_t *)malloc( sizeof( item_t ) );			// sunqueen modify
    if( unlikely( !_new ) )			// sunqueen modify
        return VLC_ENOMEM;

    _new->value = value;			// sunqueen modify
    _new->next = queue->first;			// sunqueen modify
    queue->first = _new;			// sunqueen modify

    return VLC_SUCCESS;
}

uint64_t sms_queue_avg( sms_queue_t *queue )
{
    item_t *last = queue->first;
    if( last == NULL )
        return 0;
    uint64_t sum = queue->first->value;
    for( int i = 0; i < queue->length - 1; i++ )
    {
        if( last )
        {
            last = last->next;
            if( last )
                sum += last->value;
        }
    }
    return sum / queue->length;
}

sms_stream_t * sms_get_stream_by_cat( vlc_array_t *streams, int i_cat )
{
    sms_stream_t *ret = NULL;
    int count = vlc_array_count( streams );
    assert( count >= 0 && count <= 3 );

    for( int i = 0; i < count; i++ )
    {
        ret = (sms_stream_t *)vlc_array_item_at_index( streams, i );			// sunqueen modify
        if( ret->type == i_cat )
            return ret;
    }
    return NULL;
}

int es_cat_to_index( int i_cat )
{
    switch( i_cat )
    {
        case VIDEO_ES:
            return 0;
        case AUDIO_ES:
            return 1;
        case SPU_ES:
            return 2;
        default:
            return -1;
    }
}

int index_to_es_cat( int index )
{
    switch( index )
    {
        case 0:
            return VIDEO_ES;
        case 1:
            return AUDIO_ES;
        case 2:
            return SPU_ES;
        default:
            return -1;
    }
}

bool no_more_chunks( unsigned *indexes, vlc_array_t *streams )
{
    sms_stream_t *sms = NULL;
    int count = vlc_array_count( streams );
    unsigned ck_index;
    for( int i = 0; i < count; i++ )
    {
        sms = (sms_stream_t *)vlc_array_item_at_index( streams, i );			// sunqueen modify
        ck_index = indexes[es_cat_to_index( sms->type )];
        if( ck_index < sms->vod_chunks_nb - 1 )
            return false;
    }
    return true;
}
