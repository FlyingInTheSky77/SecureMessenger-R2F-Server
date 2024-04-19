#include "../include/SessionKey.h"

SessionKey::SessionKey()
{
    mpz_init_set_str( p_mpz_mod_, "1000000000", 10 );
    mpz_init_set_str( g_mpz_, "3", 10 );
    mpz_init_set_str( intermediate_number_mpz_, "", 10 );
    session_key_ = -1;
    mpz_init( session_key_mpz_ );

    srand( time( NULL ) );
    random_number_ = randRange( 10, 100 );
    mpz_powm_ui( intermediate_number_mpz_, g_mpz_, random_number_, p_mpz_mod_ );
    char* result_in_char_ptr = new char[62];
    mpz_get_str ( result_in_char_ptr, 10, intermediate_number_mpz_ );
    QString intermediat_in_QString ( result_in_char_ptr );
    intermediate_number_ = intermediat_in_QString.toInt();
    delete[] result_in_char_ptr;
}

void SessionKey::calculateSessionKey( const int& intermediate_number_from_client )
{
    mpz_t intermediate_number_from_client_mpz;
    mpz_init( intermediate_number_from_client_mpz );
    mpz_set_ui( intermediate_number_from_client_mpz, intermediate_number_from_client );

    mpz_powm_ui( session_key_mpz_, intermediate_number_from_client_mpz, random_number_,p_mpz_mod_ );
    char * result_in_char_ptr = new char[62];
    mpz_get_str( result_in_char_ptr, 10, session_key_mpz_ );
    QString m_session_key_in_QString ( result_in_char_ptr );
    session_key_ = m_session_key_in_QString.toInt();
}

int SessionKey::getIntermediateNumberForClient()
{
    return intermediate_number_;
}

int SessionKey::getSessionKey()
{
    return session_key_;
}

int SessionKey::randRange( int low,int high )
{
    return rand() % ( high - low + 1 ) + low;

}
