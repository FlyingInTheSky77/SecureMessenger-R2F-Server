#pragma once

enum class Server_Code
{
    connection_established,
    server_stopped,

    registration_successful,
    registration_failed,
    authorization_successful,
    authorization_failed,

    message_to_recipient,
    contacts_list,
    recipient_offline,

    secure_session_server_step,

    mistake
};

enum class Client_Code
{
    registration_request,
    authorization_request,

    contacts_request,
    message_to_server,
    message_to_recipient,

    secure_session_client_step,

    mistake
};
