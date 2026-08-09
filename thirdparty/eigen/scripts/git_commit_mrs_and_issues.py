"""Search for MRs and issues related to a list of commits."""

import argparse
import json
import sys
import subprocess
import re


def find_cherry_pick_source(commit_hash: str):
    """
    For a given commit hash, find the original commit it was cherry-picked from.

    Args:
        commit_hash: The commit hash to inspect.

    Returns:
        The full hash of the original commit if found, otherwise None.
    """
    try:
        # Use 'git show' to get the full commit message for the given hash.
        # The '-s' flag suppresses the diff output.
        # The '--format=%B' flag prints only the raw commit body/message.
        commit_message = subprocess.check_output(
            ["git", "show", "-s", "--format=%B", commit_hash.strip()],
            text=True,
            stderr=subprocess.PIPE,
        ).strip()

        # This regex looks for the specific line Git adds during a cherry-pick.
        # It captures the full 40-character SHA-1 hash.
        cherry_pick_pattern = re.compile(
            r"\(cherry picked from commit ([a-f0-9]{40})\)"
        )

        # Search the entire commit message for the pattern.
        match = cherry_pick_pattern.search(commit_message)

        if match:
            # If a match is found, return the captured group (the original commit hash).
            return match.group(1)
        else:
            return None

    except subprocess.CalledProcessError as e:
        # This error occurs if the git command fails, e.g., for an invalid hash.
        print(
            f"Error processing commit '{commit_hash.strip()}': {e.stderr.strip()}",
            file=sys.stderr,
        )
        return None
    except FileNotFoundError:
        # This error occurs if the 'git' command itself isn't found.
        print(
            "Error: 'git' command not found. Please ensure Git is installed and in your PATH.",
            file=sys.stderr,
        )
        sys.exit(1)


def main():
    """
    Main function to read commit hashes from stdin and process them.
    """
    parser = argparse.ArgumentParser(
        description="A script to download all MRs from GitLab matching specified criteria."
    )
    parser.add_argument(
        "--merge_requests_file",
        type=str,
        required=True,
        help="JSON file containing all the merge request information extracted via the GitLab API.",
    )

    # E.g. git log --pretty=%H 3e819d83bf52abda16bb53565f6801df40d071f1..3.4.1
    parser.add_argument(
        "--commits",
        required=True,
        help="List of commits, '-' for stdin.",
    )
    args = parser.parse_args()

    mrs = []
    with open(args.merge_requests_file, "r") as file:
        mrs = json.load(file)
        mrs_by_commit = {}

    if args.commits == "-":
        commit_hashes = sys.stdin.readlines()
    else:
        with open(args.commits, "r") as file:
            commit_hashes = file.readlines()

    # Arrange commits by SHA.
    for mr in mrs:
        for key in ["sha", "merge_commit_sha", "squash_commit_sha"]:
            sha = mr[key]
            if sha:
                mrs_by_commit[sha] = mr

    # Find the MRs and issues related to each commit.
    info = {}
    for sha in commit_hashes:
        sha = sha.strip()
        if not sha:
            continue

        # If a cherry-pick, extract the original hash.
        sha = find_cherry_pick_source(sha) or sha
        mr = mrs_by_commit.get(sha)

        commit_info = {}
        if mr:
            commit_info["merge_request"] = mr["iid"]
            commit_info["related_issues"] = [
                issue["iid"] for issue in mr["related_issues"]
            ]
            commit_info["closes_issues"] = [
                issue["iid"] for issue in mr["closes_issues"]
            ]

        info[sha] = commit_info

    print(json.dumps(info, indent=2))


if __name__ == "__main__":
    main()
